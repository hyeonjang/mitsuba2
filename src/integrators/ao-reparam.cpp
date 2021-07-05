#include <random>
#include <enoki/stl.h>
#include <mitsuba/core/ray.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/render/integrator.h>
#include <mitsuba/render/records.h>

#include <enoki/transform.h>
#include <mitsuba/core/bsphere.h>
#include <mitsuba/core/warp.h>
#include <mitsuba/core/frame.h>
#include <mitsuba/core/fresolver.h>
#include <mitsuba/core/mmap.h>
#include <list>
#include "path-reparam-utils.h"

#define REUSE_CAMERA_RAYS 1

NAMESPACE_BEGIN(mitsuba)

/**!

.. _integrator-pathreparam:

Differentiable path tracer (:monosp:`pathreparam`)
--------------------------------------------------

.. pluginparameters::

 * - max_depth
   - |int|
   - Specifies the longest path depth in the generated output image (where -1
     corresponds to :math:`\infty`). A value of 1 will only render directly
     visible light sources. 2 will lead to single-bounce (direct-only)
     illumination, and so on. (Default: -1)
 * - rr_depth
   - |int|
   - Specifies the minimum path depth, after which the implementation will
     start to use the *russian roulette* path termination criterion. (Default: 5)
 * - dc_light_samples
   - |int|
   - Specifies the number of samples for reparameterizing direct lighting
     integrals. (Default: 4)
 * - dc_bsdf_samples
   - |int|
   - Specifies the number of samples for reparameterizing BSDFs integrals.
     (Default: 4)
 * - dc_cam_samples
   - |int|
   - Specifies the number of samples for reparameterizing pixel integrals.
     (Default: 4)
 * - conv_threshold
   - |float|
   - Specifies the BSDFs roughness threshold that activates convolutions.
     (Default: 0.15f)
 * - use_convolution
   - |bool|
   - Enable convolution for rough BSDFs. (Default: yes, i.e. |true|)
 * - kappa_conv
   - |float|
   - Specifies the kappa parameter of von Mises-Fisher distributions for BSDFs
     convolutions. (Default: 1000.f)
 * - use_convolution_envmap
   - |bool|
   - Enable convolution for environment maps. (Default: yes, i.e. |true|)
 * - kappa_conv_envmap
   - |float|
   - Specifies the kappa parameter of von Mises-Fisher distributions for
     environment map convolutions. (Default: 1000.f)
 * - use_variance_reduction
   - |bool|
   - Enable variation reduction. (Default: yes, i.e. |true|)
 * - disable_gradient_diffuse
   - |bool|
   - Disable reparameterization for diffuse scattering. (Default: no, i.e. |false|)
 * - disable_gradient_bounce
   - |int|
   - Disable reparameterization after several scattering events. (Default: 10)

This integrator implements the reparameterization technique described in the
`article <https://rgl.epfl.ch/publications/Loubet2019Reparameterizing>`_
"Reparameterizing discontinuous integrands for differentiable rendering".
It is based on the integrator :ref:`path <integrator-path>` and it applies
reparameterizations to rendering integrals in order to account for discontinuities
when pixel values are differentiated using GPU modes and the Python API.

This plugin supports environment maps and area lights with the plugin
:ref:`smootharea <emitter-smootharea>`, which is similar to the plugin
:ref:`area <emitter-area>` with smoothly decreasing radiant exitance at the
borders of the area light geometry to avoid discontinuities. The area light
geometry should be flat and it should have valid uv coordinates (see
:ref:`smootharea <emitter-smootharea>` for details). Other light
sources will lead to incorrect partial derivatives. Large area lights also
result in significant bias since the convolution technique described in the
paper is only applied to environment maps and rough/diffuse BSDF integrals.

Another limitation of this implementation is memory usage on the GPU: automatic
differentiation for an entire path tracer typically requires several GB of GPU
memory. The rendering must sometimes be split into various rendering passes with
small sample counts in order to fit into GPU memory.

.. note:: This integrator does not handle participating media

 */

template <typename Float, typename Spectrum>
class AOReparamIntegrator : public SamplingIntegrator<Float, Spectrum> {
public:
    MTS_IMPORT_BASE(SamplingIntegrator)
    MTS_IMPORT_TYPES(Scene, Sampler, Medium)

    AOReparamIntegrator(const Properties &props) : Base(props) {
        m_shading_samples   = props.size_("shading_samples", 4);
        m_ray_length        = props.float_("ray_length", -1);
        m_dc_cam_samples   = props.size_("dc_cam_samples",  4);
        m_conv_threshold   = props.float_("conv_threshold",  0.15f);
        m_kappa_conv       = props.float_("kappa_conv",      1000.f);
        m_kappa_conv_envmap      = props.float_("kappa_conv_envmap",     100000.f);
        m_use_convolution        = props.bool_("use_convolution",        true);
        m_use_convolution_envmap = props.bool_("use_convolution_envmap", true);
        m_use_variance_reduction = props.bool_("use_variance_reduction", true);
        m_disable_gradient_diffuse = props.bool_("disable_gradient_diffuse", false);
        m_disable_gradient_bounce  = props.size_("disable_gradient_bounce", 1000);

        Log(Debug, "Changes of variables in pixel integrals using %i samples",
            m_dc_cam_samples);
        Log(Debug, "Changes of variables using convolution if roughness > %f",
            m_conv_threshold);
        Log(Debug, "Convolutions using kernel with kappa = %f",
            m_kappa_conv);
        Log(Debug, "Variance reduction %s",
            m_use_variance_reduction ? "enabled" : "disabled");
        Log(Debug, "Convolutions %s",
            m_use_convolution ? "enabled" : "disabled");
        Log(Debug, "Convolutions for envmap %s",
            m_use_convolution_envmap ? "enabled" : "disabled");
        Log(Debug, "Gradient of diffuse reflections %s",
            m_disable_gradient_diffuse ? "disabled" : "enabled");
        Log(Debug, "Disable gradients after bounce %i", m_disable_gradient_bounce);
        Log(Debug, "Reusing camera samples is %s",
            REUSE_CAMERA_RAYS ? "enabled" : "disabled");
    }

    std::pair<Spectrum, Mask> sample(const Scene *scene,
                                     Sampler *sampler,
                                     const RayDifferential3f &primary_ray_,
                                     const Medium * /* medium */,
                                     Float * /* aovs */,
                                     Mask active_primary) const override {
        if constexpr (is_cuda_array_v<Float>) {
            RayDifferential3f primary_ray = primary_ray_;

            // Estimate kappa for the convolution of pixel integrals, based on ray
            // differentials.
            Float angle = acos(min(dot(primary_ray.d_x, primary_ray.d), dot(primary_ray.d_y, primary_ray.d)));
            Float target_mean_cos = min(cos(angle * 0.4f /*arbitrary*/), Float(1.f - 1e-7f));

            // The vMF distribution has an analytic expression for the mean cosine:
            //                  mean = 1 + 2/(exp(2*k)-1) - 1/k.
            // For large values of kappa, 1-1/k is a precise approximation of this
            // function. It can be inverted to find k from the mean cosine.
            Float kappa_camera = Float(1.f) / (Float(1.f) - target_mean_cos);

            const size_t nb_pimary_rays = slices(primary_ray.d);
            const UInt32 arange_indices = arange<UInt32>(nb_pimary_rays);

            Spectrum result(0.f);

            // ---------------- Convolution of pixel integrals -------------

            // Detect discontinuities in a small vMF kernel around each ray.

            std::vector<RayDifferential3f>      rays(m_dc_cam_samples);
            std::vector<SurfaceInteraction3f>   sis(m_dc_cam_samples);

            Frame<Float> frame_input = Frame<Float>(primary_ray.d);

            Vector3f dir_conv_0, dir_conv_1;

            // Sample the integrals and gather intersections
            for (size_t cs = 0; cs < m_dc_cam_samples; cs++) 
            {
                Vector3f vMF_sample_cs = warp::square_to_von_mises_fisher<Float>(sampler->next_2d(active_primary), kappa_camera);
                Vector3f dir_conv_cs = frame_input.to_world(vMF_sample_cs);

                primary_ray.d = dir_conv_cs;
                sis[cs] = scene->ray_intersect(primary_ray, HitComputeMode::Least, active_primary);
                sis[cs].compute_differentiable_shape_position(active_primary);

                rays[cs] = RayDifferential(primary_ray);

                // Keep two directions for creating pairs of paths.
                // We choose the last samples since they have less
                // chances of being used in the estimation of the
                // discontinuity.
                if (cs==m_dc_cam_samples-2) dir_conv_0 = dir_conv_cs;
                if (cs==m_dc_cam_samples-1) dir_conv_1 = dir_conv_cs;
            }

            Point3f discontinuity = estimate_discontinuity(rays, sis, active_primary);
            Vector3f discontinuity_dir = normalize(discontinuity - primary_ray.o);

            // The following rotation seems to be the identity transformation, but it actually
            // changes the partial derivatives.

            // Create the differentiable rotation
            Vector3f axis = cross<Vector3f>(detach(discontinuity_dir), discontinuity_dir);
            Float cosangle = dot(discontinuity_dir, detach(discontinuity_dir));
            Transform4f rotation = rotation_from_axis_cosangle(axis, cosangle);

            // Make pairs of rays (reuse 2 samples) and apply rotation
            Spectrum throughput(1.f);

#if !REUSE_CAMERA_RAYS
            // Resample two rays. This tends to add bias on silhouettes.
            dir_conv_0 = frame_input.to_world(
                warp::square_to_von_mises_fisher<Float, Float>(
                    sampler->next_2d(active_primary), kappa_camera));
            dir_conv_1 = frame_input.to_world(
                warp::square_to_von_mises_fisher<Float, Float>(
                    sampler->next_2d(active_primary), kappa_camera));
#endif

            // NOTE: here we detach because the rays will be passed to Optix, no need for autodiff
            Vector ray_d_0 = rotation.transform_affine(detach(dir_conv_0));
            Vector ray_d_1 = rotation.transform_affine(detach(dir_conv_1));

            Vector3f    ray_d = concatD(ray_d_0, ray_d_1);
            Point3f     ray_o = makePairD<Point3f>(primary_ray.o);
            Wavelength  ray_w = makePairD(primary_ray.wavelengths);

            Ray3f ray = Ray3f(ray_o, ray_d, 0.0, ray_w);

            Mask active(true);
            set_slices(active, nb_pimary_rays * 2);

            // Recompute differentiable pdf
            Float vMF_pdf_diff_0 = warp::square_to_von_mises_fisher_pdf<Float, Float>(frame_input.to_local(ray_d_0), kappa_camera);
            Float vMF_pdf_diff_1 = warp::square_to_von_mises_fisher_pdf<Float, Float>(frame_input.to_local(ray_d_1), kappa_camera);
            Float vMF_pdf_diff   = concatD<Float>(vMF_pdf_diff_0, vMF_pdf_diff_1);

            // Apply differentiable weight and keep for variance reduction
            throughput *= vMF_pdf_diff / detach(vMF_pdf_diff); // NOTE: detach here so we only divide the gradient by the pdf
            Float current_weight = vMF_pdf_diff / detach(vMF_pdf_diff);

            // ---------------------- First intersection ----------------------
            Float ray_length = m_ray_length;
            if( any_or<true>(ray_length<0) ) ray_length = scene->bbox().bounding_sphere().radius * Float(0.5f);
            
            auto si = scene->ray_intersect(ray, HitComputeMode::Differentiable, active);
            Mask valid_ray_pair = si.is_valid();
            Mask valid_ray = gather<Mask>(valid_ray_pair, arange_indices)||gather<Mask>(valid_ray_pair, arange_indices + nb_pimary_rays);

            std::vector<Point2f> samples(m_shading_samples, samplePair2D(active, sampler));
            for (size_t depth=1;; ++depth) 
            {
                active &= si.is_valid();

                if (none(active)||(uint32_t)depth>=(uint32_t)m_shading_samples) break;

                auto d = si.to_world(warp::square_to_cosine_hemisphere(samples[depth-1]));

                // check reusing geometry intersection
                Float component_sample = samplePair1D(active, sampler);
                Mask convolution = Mask(m_use_convolution) && active; // sample_main_bs.sampled_roughness

                Frame<Float> frame_main_sh(d);
                std::vector<Vector3f> ds_sh(m_shading_samples);

                for(size_t sh=0; sh<m_shading_samples; sh++)
                {
                    Point2f samples = sample2D(active, sampler);
                    Vector3f sample_sh = warp::square_to_von_mises_fisher<Float>(samples, m_kappa_conv);
                    sample_sh = frame_main_sh.to_world(sample_sh);

                    auto sample_sh_noconv = si.to_world(warp::square_to_cosine_hemisphere(samples));
                    ds_sh[sh] = select(convolution, sample_sh, sample_sh_noconv);
                }

                std::vector<RayDifferential3f> rays_sh(m_shading_samples);
                std::vector<SurfaceInteraction3f> sis_sh(m_shading_samples);

                Mask use_reparam_sh(false);
                for(size_t sh=0; sh<m_shading_samples; sh++)
                {
                    rays_sh[sh] = si.spawn_ray(si.to_world(ds_sh[sh]));
                    sis_sh[sh] = scene->ray_intersect(rays_sh[sh], HitComputeMode::Least, active);
                    sis_sh[sh].compute_differentiable_shape_position(active);
                    use_reparam_sh = use_reparam_sh || (active && neq(sis_sh[sh].shape, nullptr));
                }

                if (m_disable_gradient_diffuse) {
                    use_reparam_sh &= !convolution;
                    current_weight = select(use_reparam_sh, current_weight, detach(current_weight));
                }

                Point3f discontinuity_sh = estimate_discontinuity(rays_sh, sis_sh, active);

                Vector3f direction_diff = normalize(discontinuity_sh - si.p);
                Vector3f discontinuity_sh_detach = detach(discontinuity_sh);
                Vector3f direction_detach = normalize(discontinuity_sh_detach - si.p);

                Vector3f axis_sh = cross(direction_detach, direction_diff);
                Float cosangle_sh = dot(direction_diff, direction_detach);
                Transform4f rotation_sh = rotation_from_axis_cosangle(axis_sh, cosangle_sh);

                auto sample_sh = d;
                sample_sh = ds_sh[0];

                sample_sh[use_reparam_sh] = si.to_local(rotation_sh.transform_affine(si.to_world(detach(sample_sh))));

                Ray3f shadow_ray(si.p, sample_sh, math::RayEpsilon<Float>, ray_length);
                SurfaceInteraction3f si_sh = scene->ray_intersect(shadow_ray, HitComputeMode::Differentiable, active); //Least? differential?
                
                Float pdf_conv_new_dir = warp::square_to_von_mises_fisher_pdf<Float>(frame_main_sh.to_local(sample_sh), m_kappa_conv);

                // correction term for the convolution (otherwise less energy
                // at grazing angles)
                Float cosangle_vmf = sample_sh.z();
                Float correction_factor = m_vmf_hemisphere.eval(m_kappa_conv, cosangle_vmf, convolution);
                
                //active &= si_sh.is_valid();
                Mask active_sh = active && neq(si_sh.is_valid(), true);
                si_sh.compute_differentiable_shape_position(active_sh);
               
                Spectrum sampling(0.f);
                sampling[active_sh] = current_weight * throughput * select(active_sh, Spectrum(1.f), Spectrum(0.f));//1.f * 1.f * active; 

                Spectrum sampling_0 = gather<Spectrum>(sampling, arange_indices);//select(active_s0, Spectrum(1.f), Spectrum(0.f));
                Spectrum sampling_1 = gather<Spectrum>(sampling, arange_indices+nb_pimary_rays);//select(active_s1, Spectrum(1.f), Spectrum(0.f));

                Float weights_0 = gather<Float>(current_weight, arange_indices);
                Float weights_1 = gather<Float>(current_weight, arange_indices + nb_pimary_rays);

                if (depth >= m_disable_gradient_bounce) 
                {
                    result += detach(sampling_0) * 0.5f;
                    result += detach(sampling_1) * 0.5f;
                } 
                else if (m_use_variance_reduction) 
                {
                    weights_0 = select(abs(weights_0) < 0.00001f, Float(1.f), weights_0)* 0.5f;
                    weights_1 = select(abs(weights_1) < 0.00001f, Float(1.f), weights_1)* 0.5f;
                        // Variance reduction, assumption that contribution = weight * constant
                    result += (sampling_0 - sampling_1 / weights_1 * (weights_0 - detach(weights_0)))* 0.5f; // NOTE: detach here so to only add `e_1/w_1*w_0` to the gradient (only try to reduce the variance of the gradient)
                    result += (sampling_1 - sampling_0 / weights_0 * (weights_1 - detach(weights_1)))* 0.5f;
                } 
                else 
                {
                    result += sampling_0* 0.5f;
                    result += sampling_1* 0.5f;
                }
                //si = std::move(si_sh);
            }
            result /= Float(m_shading_samples)*0.5f;
            return { result, valid_ray };
        } else {
            ENOKI_MARK_USED(scene);
            ENOKI_MARK_USED(sampler);
            ENOKI_MARK_USED(primary_ray_);
            ENOKI_MARK_USED(active_primary);
            Throw("AOReparamIntegrator: currently this integrator must be run on the GPU.");
            return {Spectrum(0.f), Mask(false)};
        }
    }

    std::string to_string() const override {
        return tfm::format("AOIntegrator[\n"
            "  shading_samples = %i,\n"
            "  ray_length = %i\n"
            "]", m_shading_samples, m_ray_length);
    }

    MTS_DECLARE_CLASS()

protected:
    mitsuba::Transform<Vector4f> rotation_from_axis_cosangle(Vector3f axis, Float cosangle) const {
        Float ax = axis.x(),
              ay = axis.y(),
              az = axis.z();
        Float axy = ax * ay,
              axz = ax * az,
              ayz = ay * az;

        Matrix3f ux(0.f, -az,  ay,
                     az, 0.f, -ax,
                    -ay,  ax, 0.f);

        Matrix3f uu(sqr(ax),     axy,    axz,
                        axy, sqr(ay),    ayz,
                        axz,     ayz, sqr(az));

        Matrix3f R = identity<Matrix3f>() * cosangle + ux + rcp(1 + cosangle) * uu;

        return mitsuba::Transform<Vector4f>(Matrix4f(R));
    };

    Point3f estimate_discontinuity(const std::vector<RayDifferential3f> &rays,
                                   const std::vector<SurfaceInteraction3f> &sis,
                                   const Mask &/*mask*/) const {

        using Matrix = enoki::Matrix<Float, 3>;

        size_t nb_samples = rays.size();

        if (rays.size() < 2 || rays.size() != sis.size())
            Throw("PathReparamIntegrator::estimate_discontinuity: invalid number of samples for discontinuity estimation");

        Point3f ray0_p_attached = sis[0].p;
        Vector3f ray0_n = sis[0].n;

        UInt32 is_ray1_hit_uint = select(neq(sis[1].shape, nullptr), UInt32(1), UInt32(0));
        Point3f ray1_p_attached = sis[1].p;
        Vector3f ray1_n = sis[1].n;
        Vector3f ray1_d = rays[1].d;

        for (size_t i = 2; i < nb_samples; i++) {
            Mask diff  = neq(sis[0].shape, sis[i].shape);
            Mask i_hit = neq(sis[i].shape, nullptr);
            is_ray1_hit_uint = select(diff, select(i_hit, UInt32(1), UInt32(0)), is_ray1_hit_uint);
            ray1_p_attached  = select(diff, sis[i].p, ray1_p_attached);
            ray1_n           = select(diff, sis[i].n, ray1_n);
            ray1_d           = select(diff, rays[i].d, ray1_d);
        }

        Mask is_ray1_hit = is_ray1_hit_uint > 0;

        // Guess occlusion for pairs of samples

        Point3f res(0.f);

        // if only one hit: return this hit
        Mask only_hit_0 = neq(sis[0].shape, nullptr) && !is_ray1_hit;
        res[only_hit_0] = ray0_p_attached;

        Mask only_hit_1 = is_ray1_hit && eq(sis[0].shape, nullptr);
        res[only_hit_1] = ray1_p_attached;

        Mask has_two_hits = neq(sis[0].shape, nullptr) && is_ray1_hit;

        // Compute occlusion between planes and hitpoints: sign of
        // dot(normal, hitpoint - hitpoint). Test if the origin of the rays
        // is on the same side as the other hit.
        Float occ_plane_0 =
            dot(ray0_n, ray1_p_attached - ray0_p_attached) *
            dot(ray0_n, rays[0].o - ray0_p_attached);
        Float occ_plane_1 = dot(ray1_n, ray0_p_attached - ray1_p_attached) *
                            dot(ray0_n, rays[0].o - ray0_p_attached);

        Mask plane_0_occludes_1 = has_two_hits && (occ_plane_0 < 0.f);
        Mask plane_1_occludes_0 = has_two_hits && (occ_plane_1 < 0.f);

        Mask simple_occluder_0 = plane_0_occludes_1 && !plane_1_occludes_0;
        Mask simple_occluder_1 = plane_1_occludes_0 && !plane_0_occludes_1;
        Mask plane_intersection = has_two_hits  && !simple_occluder_1 && !simple_occluder_0;

        /* simple_occluder */

        res[simple_occluder_0] = ray0_p_attached;
        res[simple_occluder_1] = ray1_p_attached;

        /* same_normals */

        Mask same_normals = plane_intersection  && abs(dot(ray0_n, ray1_n)) > 0.99f;
        plane_intersection &= !same_normals;
        res[same_normals] = ray0_p_attached;

        /* plane_intersection */

#if 1
        // Compute the intersection between 3 planes:
        // 2 planes defined by the ray intersections and
        // the normals at these points, and 1 plane containing
        // the ray directions.

        Vector3f N0 = ray0_n;
        Vector3f N1 = ray1_n;
        Vector3f P0 = ray0_p_attached;
        Vector3f P1 = ray1_p_attached;

        // Normal of the third plane, defined using
        // attached positions (this prevents bad correlations
        // between the displacement of the intersection and
        // the sampled positions)

        Vector3f N = cross(P0 - rays[0].o, P1 - rays[0].o);
        Float norm_N = norm(N);

        // Set a default intersection if the problem is ill-defined
        res[plane_intersection] = ray0_p_attached;

        Mask invertible = plane_intersection && norm_N > 0.001f;

        Matrix A = Matrix::from_rows(N0, N1, N);
        Float b0 =  dot(P0, N0);
        Float b1 =  dot(P1, N1);
        Float b2 =  dot(rays[0].o, N);
        Vector3f B(b0, b1, b2);
        Matrix invA = enoki::inverse(A);
        res[invertible] = invA * B;
#else
        // Simply choose one of the intersections.
        // This is a good strategy in many situations.
        res[plane_intersection] = ray0_p_attached;
#endif

        return res;

    }

private:
    size_t m_shading_samples;
    Float  m_ray_length;

    size_t m_disable_gradient_bounce;
    size_t m_dc_cam_samples;

    ScalarFloat m_conv_threshold;
    ScalarFloat m_kappa_conv;
    ScalarFloat m_kappa_conv_envmap;
    bool m_use_variance_reduction;
    bool m_use_convolution;
    bool m_use_convolution_envmap;
    bool m_disable_gradient_diffuse;

    VMFHemisphereIntegral<Float> m_vmf_hemisphere;
};

MTS_IMPLEMENT_CLASS_VARIANT(AOReparamIntegrator, SamplingIntegrator);
MTS_EXPORT_PLUGIN(AOReparamIntegrator, "Differentiable Ambient Occlusion Tracer integrator");
NAMESPACE_END(mitsuba)
