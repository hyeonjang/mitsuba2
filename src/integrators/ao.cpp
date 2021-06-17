#include <enoki/stl.h>
#include <mitsuba/render/integrator.h>
#include <mitsuba/render/bsdf.h>
#include <mitsuba/core/properties.h>

#include <mitsuba/core/bsphere.h>
#include <mitsuba/core/warp.h>
#include <mitsuba/core/ray.h>

NAMESPACE_BEGIN(mitsuba)

/*! \plugin{ao}{Ambient occlusion integrator}
 * \order{0}
 * \parameters{
 *     \parameter{shadingSamples}{\Integer}{Specifies the number of
 *         shading samples that should be computed per primary ray
 *         \default{1}}
 *
 *     \parameter{rayLength}{\Float}{Specifies the world-space length of the
 *         ambient occlusion rays that will be cast. \default{\code{-1}, i.e. automatic}}.
 * }
 * \renderings{
 *    \rendering{A view of the scene on page \pageref{fig:rungholt}, rendered using
 *               the Ambient Occlusion integrator}{integrator_ao}
 *    \rendering{A corresponding rendering created using the standard \pluginref{path} tracer}{integrator_ao_path}
 * }
 * Ambient Occlusion is a simple non-photorealistic rendering technique that simulates the exposure of an object
 * to uniform illumination incident from all direction. It produces approximate shadowing between closeby
 * objects, as well as darkening in corners, creases, and cracks. The scattering models associated with objects
 * in the scene are ignored.
 */

template <typename Float, typename Spectrum>
class AOIntegrator : public SamplingIntegrator<Float, Spectrum> {
public:
    MTS_IMPORT_BASE(SamplingIntegrator)
    MTS_IMPORT_TYPES(Scene, Sampler, Medium, BSDF, BSDFPtr)

    AOIntegrator(const Properties &props) : Base(props) {
        m_shading_samples  = props.size_("shading_samples", 1);
        m_ray_length = props.float_("ray_length", -1);
    }

    Float mis_weight(Float pdf_a, Float pdf_b) const {
        pdf_a *= pdf_a;
        pdf_b *= pdf_b;
        return select(pdf_a > 0.f, pdf_a / (pdf_a + pdf_b), 0.f);
    }

    std::pair<Spectrum, Mask> sample(const Scene *scene,
                                    Sampler *sampler,
                                    const RayDifferential3f &ray,
                                    const Medium * /* medium */,
                                    Float * /* aovs */,
                                    Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::SamplingIntegratorSample, active);
    
        // There is no preprocessing function in mitsuba2. If you want, uses render_sample()
        Float ray_length = m_ray_length;
        if( any_or<true>(ray_length<0) ) ray_length = scene->bbox().bounding_sphere().radius * Float(0.5f);

        SurfaceInteraction3f si = scene->ray_intersect(ray, active);
        Mask valid_ray = si.is_valid();

        Spectrum result(0.f);

        size_t num = m_shading_samples;
        std::vector<Point2f> samples(num, sampler->next_2d());
        for( auto& sample : samples ) 
        {
            auto d = si.to_world(warp::square_to_cosine_hemisphere(sample));

            active &= si.is_valid();

            Ray3f shadow_ray(si.p, d, math::RayEpsilon<Float>, ray_length);
            SurfaceInteraction3f si_sh = scene->ray_intersect(shadow_ray, active);

            Mask active_sh = neq(si_sh.is_valid(), true);
            result += select(active_sh, Float(1.f), Float(0.f));
        }
        result /= Float(num);

        return { result, active };
    }

    std::string to_string() const override {
        return tfm::format("AOIntegrator[\n"
            "  shading_samples = %i,\n"
            "  ray_length = %i\n"
            "]", m_shading_samples, m_ray_length);
    }

    MTS_DECLARE_CLASS()
private:
    size_t m_shading_samples;
    mutable Float m_ray_length;
};

MTS_IMPLEMENT_CLASS_VARIANT(AOIntegrator, SamplingIntegrator)
MTS_EXPORT_PLUGIN(AOIntegrator, "Ambient occlusion integrator");
NAMESPACE_END(mitsuba)