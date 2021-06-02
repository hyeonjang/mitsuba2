#include <mitsuba/core/bitmap.h>
#include <mitsuba/core/bsphere.h>
#include <mitsuba/core/distr_2d.h>
#include <mitsuba/core/fresolver.h>
#include <mitsuba/core/plugin.h>
#include <mitsuba/render/emitter.h>
#include <mitsuba/render/scene.h>
#include <mitsuba/render/texture.h>
#include <mitsuba/render/srgb.h>

NAMESPACE_BEGIN(mitsuba)

/**!

.. _emitter-envmap:

Environment emitter (:monosp:`envmap`)
--------------------------------------

.. pluginparameters::

 * - filename
   - |string|
   - Filename of the radiance-valued input image to be loaded; must be in latitude-longitude format.
 * - scale
   - |Float|
   - A scale factor that is applied to the radiance values stored in the input image. (Default: 1.0)
 * - to_world
   - |transform|
   - Specifies an optional emitter-to-world transformation.  (Default: none, i.e. emitter space = world space)

This plugin provides a HDRI (high dynamic range imaging) environment map,
which is a type of light source that is well-suited for representing "natural"
illumination.

The implementation loads a captured illumination environment from a image in
latitude-longitude format and turns it into an infinitely distant emitter. The
conventions of this mapping are shown in this image:

.. subfigstart::
.. subfigure:: ../../resources/data/docs/images/emitter/emitter_envmap_example.jpg
   :caption: The museum environment map by Bernhard Vogl that is used in
             many example renderings in this documentation.
.. subfigure:: ../../resources/data/docs/images/emitter/emitter_envmap_axes.jpg
   :caption: Coordinate conventions used when mapping the input image onto the sphere.
.. subfigend::
   :label: fig-envmap-mapping

The plugin can work with all types of images that are natively supported by Mitsuba
(i.e. JPEG, PNG, OpenEXR, RGBE, TGA, and BMP). In practice, a good environment
map will contain high-dynamic range data that can only be represented using the
OpenEXR or RGBE file formats.
High quality free light probes are available on
`Paul Debevec's <http://gl.ict.usc.edu/Data/HighResProbes>`_ and
`Bernhard Vogl's <http://dativ.at/lightprobes/>`_ websites.

 */
template <typename Float, typename Spectrum>
class TiledEnvironmentMapEmitter final : public Emitter<Float, Spectrum> {
public:
    MTS_IMPORT_BASE(Emitter, m_flags, m_world_transform)
    MTS_IMPORT_TYPES(Scene, Shape, Texture)

    using Warp = Hierarchical2D<Float, 0>;

    TiledEnvironmentMapEmitter(const Properties &props) : Base(props) {
        /* Until `set_scene` is called, we have no information
           about the scene and default to the unit bounding sphere. */
        m_bsphere = ScalarBoundingSphere3f(ScalarPoint3f(0.f), 1.f);
        m_tile_count = ScalarVector2u(props.int_("row"), props.int_("col"));
        m_datas = std::vector<DynamicBuffer<Float>>(m_tile_count.x() * m_tile_count.y());

        FileResolver *fs = Thread::thread()->file_resolver();
        fs::path file_path = fs->resolve(props.string("filename"));

        ref<Bitmap> bitmap = new Bitmap(file_path);

        /* Convert to linear RGBA float bitmap, will undergo further
           conversion into coefficients of a spectral upsampling model below */
        bitmap = bitmap->convert(Bitmap::PixelFormat::RGBA, struct_type_v<ScalarFloat>, false);
        m_filename = file_path.filename().string();

        std::unique_ptr<ScalarFloat[]> luminance(new ScalarFloat[bitmap->pixel_count()]);

        ScalarFloat *ptr     = (ScalarFloat *) bitmap->data(),
                    *lum_ptr = (ScalarFloat *) luminance.get();

        for (size_t y = 0; y < bitmap->size().y(); ++y) {
            ScalarFloat sin_theta =
                std::sin(y / ScalarFloat(bitmap->size().y() - 1) * math::Pi<ScalarFloat>);

            for (size_t x = 0; x < bitmap->size().x(); ++x) {
                ScalarColor3f rgb = load_unaligned<ScalarVector3f>(ptr);
                ScalarFloat lum   = mitsuba::luminance(rgb);

                ScalarVector4f coeff;
                if constexpr (is_monochromatic_v<Spectrum>) {
                    coeff = ScalarVector4f(lum, lum, lum, 1.f);
                } else if constexpr (is_rgb_v<Spectrum>) {
                    coeff = concat(rgb, ScalarFloat(1.f));
                } else {
                    static_assert(is_spectral_v<Spectrum>);
                    /* Evaluate the spectral upsampling model. This requires a
                       reflectance value (colors in [0, 1]) which is accomplished here by
                       scaling. We use a color where the highest component is 50%,
                       which generally yields a fairly smooth spectrum. */
                    ScalarFloat scale = hmax(rgb) * 2.f;
                    ScalarColor3f rgb_norm = rgb / std::max((ScalarFloat) 1e-8, scale);
                    coeff = concat((ScalarColor3f) srgb_model_fetch(rgb_norm), scale);
                }

                *lum_ptr++ = lum * sin_theta;
                store_unaligned(ptr, coeff);
                ptr += 4;
            }
        }

        // tile partitioning start
        m_resolution = bitmap->size();
        auto tile_size  = m_resolution/m_tile_count; 
        ref<Bitmap> tile = new Bitmap(Bitmap::PixelFormat::RGBA, struct_type_v<ScalarFloat>, tile_size);
        tile->clear();
        for(uint32_t j=0; j<m_tile_count.y(); j++)
        {
            for(uint32_t i=0; i<m_tile_count.x(); i++)
            {
                tile->accumulate(bitmap, {i*tile_size.x(), j*tile_size.y()}, {0, 0}, tile_size);
                auto index = i+j*m_tile_count.x();
                m_datas[index] = DynamicBuffer<Float>::copy(tile->data(), hprod(tile_size) * 4);
                auto path = std::to_string(index) + ".exr";
                tile->clear();
            }
        }
        // tile partitioning end 

        m_scale = props.float_("scale", 1.f);
        m_warp = Warp(luminance.get(), m_resolution);
        m_d65 = Texture::D65(1.f);
        m_flags = EmitterFlags::Infinite | EmitterFlags::SpatiallyVarying;
    }

    void set_scene(const Scene *scene) override {
        m_bsphere = scene->bbox().bounding_sphere();
        m_bsphere.radius = max(math::RayEpsilon<Float>, m_bsphere.radius * (1.f + math::RayEpsilon<Float>));
    }

    Spectrum eval(const SurfaceInteraction3f &si, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::EndpointEvaluate, active);

        Vector3f v = m_world_transform->eval(si.time, active).inverse().transform_affine(-si.wi);

        /* Convert to latitude-longitude texture coordinates */
        Point2f uv = Point2f(atan2(v.x(), -v.z()) * math::InvTwoPi<Float>,
                             safe_acos(v.y()) * math::InvPi<Float>);
        uv -= floor(uv);

        return unpolarized<Spectrum>(eval_spectrum(uv, si.wavelengths, active));
    }

    std::pair<Ray3f, Spectrum> sample_ray(Float /* time */, Float /* wavelength_sample */,
                                          const Point2f & /* sample2 */,
                                          const Point2f & /* sample3 */,
                                          Mask /* active */) const override {
        NotImplementedError("sample_ray");
    }

    std::pair<DirectionSample3f, Spectrum>
    sample_direction(const Interaction3f &it, const Point2f &sample, Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::EndpointSampleDirection, active);

        auto [uv, pdf] = m_warp.sample(sample);

        Float theta = uv.y() * math::Pi<Float>;
        Float phi   = uv.x() * (2.f * math::Pi<Float>);

        Vector3f d = math::sphdir(theta, phi);
        d = Vector3f(d.y(), d.z(), -d.x());

        Float dist = 2.f * m_bsphere.radius;

        Float inv_sin_theta =
            safe_rsqrt(max(sqr(d.x()) + sqr(d.z()), sqr(math::Epsilon<Float>)));

        d = m_world_transform->eval(it.time, active).transform_affine(d);

        DirectionSample3f ds;
        ds.p      = it.p + d * dist;
        ds.n      = -d;
        ds.uv     = uv;
        ds.time   = it.time;
        ds.pdf = select(pdf > 0.f, pdf * inv_sin_theta * (1.f / (2.f * sqr(math::Pi<Float>))), 0.f);
        ds.delta  = false;
        ds.object = this;
        ds.d      = d;
        ds.dist   = dist;

        return std::make_pair(
            ds,
            unpolarized<Spectrum>(eval_spectrum(uv, it.wavelengths, active)) / ds.pdf
        );
    }

    Float pdf_direction(const Interaction3f &it, const DirectionSample3f &ds,
                        Mask active) const override {
        MTS_MASKED_FUNCTION(ProfilerPhase::EndpointEvaluate, active);

        Vector3f d = m_world_transform->eval(it.time, active)
                         .inverse()
                         .transform_affine(ds.d);

        /* Convert to latitude-longitude texture coordinates */
        Point2f uv = Point2f(atan2(d.x(), -d.z()) * math::InvTwoPi<Float>,
                             safe_acos(d.y()) * math::InvPi<Float>);
        uv -= floor(uv);

        Float inv_sin_theta =
            safe_rsqrt(max(sqr(d.x()) + sqr(d.z()), sqr(math::Epsilon<Float>)));
        return m_warp.eval(uv) * inv_sin_theta * (1.f / (2.f * sqr(math::Pi<Float>)));
    }

    ScalarBoundingBox3f bbox() const override {
        /* This emitter does not occupy any particular region
           of space, return an invalid bounding box */
        return ScalarBoundingBox3f();
    }

    void traverse(TraversalCallback *callback) override {
        callback->put_parameter("scale", m_scale);
        callback->put_parameter("resolution", m_resolution);
    }

    void parameters_changed(const std::vector<std::string> &keys = {}) override {
        if (keys.empty() || string::contains(keys, "data")) {
            
            for(auto& data : m_datas) data.managed();

            std::unique_ptr<ScalarFloat[]> luminance(new ScalarFloat[hprod(m_resolution)]);

            ScalarFloat *ptr     = (ScalarFloat *) m_datas.data(),
                        *lum_ptr = (ScalarFloat *) luminance.get();


            for (size_t y = 0; y < m_resolution.y(); ++y) {
                ScalarFloat sin_theta =
                    std::sin(y / ScalarFloat(m_resolution.y() - 1) * math::Pi<ScalarFloat>);

                for (size_t x = 0; x < m_resolution.x(); ++x) {
                    ScalarVector4f coeff = load<ScalarVector4f>(ptr);
                    ScalarFloat lum;

                    if constexpr (is_monochromatic_v<Spectrum>) {
                        lum = coeff.x();
                    } else if constexpr (is_rgb_v<Spectrum>) {
                        lum = mitsuba::luminance(ScalarColor3f(head<3>(coeff)));
                    } else {
                        static_assert(is_spectral_v<Spectrum>);
                        lum = srgb_model_mean(head<3>(coeff)) * coeff.w();
                    }

                    *lum_ptr++ = lum * sin_theta;
                    ptr += 4;
                }
            }

            m_warp = Warp(luminance.get(), m_resolution);
        }
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "PixelEnvironmentMapEmitter[" << std::endl
            << "  filename = \"" << m_filename << "\"," << std::endl
            << "  resolution = \"" << m_resolution << "\"," << std::endl
            << "  tile_count = \"" << m_tile_count << "\"," << std::endl
            << "  bsphere = " << string::indent(m_bsphere) << std::endl
            << "]";
        return oss.str();
    }

protected:
    UnpolarizedSpectrum eval_spectrum(Point2f uv, const Wavelength &wavelengths, Mask active) const {
        uv *= Vector2f(m_resolution - 1u);
        ScalarVector2f tile_size  = ScalarVector2f(m_resolution)/m_tile_count; 

        Point2u pos = min(Point2u(uv), m_resolution - 2u);
        Point2u tile_2d_idx = Point2u(pos/tile_size); 

        Point2f w1 = uv - Point2f(pos);
        Point2f w0 = 1.f - w1;

        const uint32_t tile_width = tile_size.x();

        UInt32 tmp = tile_2d_idx.x() + tile_2d_idx.y() * m_tile_count.x(); 
        size_t tile_1d_idx = slice(tmp, 0); // only works on gpu_autodiff_rgb without bug


        Point2u pixel_2d_idx = pos - tile_2d_idx*tile_size;;
        UInt32  pixel_1d_idx = pixel_2d_idx.x() + pixel_2d_idx.y()*tile_width;

        auto& data = m_datas[tile_1d_idx];
        Vector4f v00 = gather<Vector4f>(data, pixel_1d_idx, active);
        Vector4f v10 = gather<Vector4f>(data, pixel_1d_idx + 1, active);
        Vector4f v01 = gather<Vector4f>(data, pixel_1d_idx + tile_width, active);
        Vector4f v11 = gather<Vector4f>(data, pixel_1d_idx + tile_width + 1, active);

        if constexpr (is_spectral_v<Spectrum>) {
            UnpolarizedSpectrum s00, s10, s01, s11, s0, s1, s;
            Float f0, f1, f;

            s00 = srgb_model_eval<UnpolarizedSpectrum>(head<3>(v00), wavelengths);
            s10 = srgb_model_eval<UnpolarizedSpectrum>(head<3>(v10), wavelengths);
            s01 = srgb_model_eval<UnpolarizedSpectrum>(head<3>(v01), wavelengths);
            s11 = srgb_model_eval<UnpolarizedSpectrum>(head<3>(v11), wavelengths);

            s0  = fmadd(w0.x(), s00, w1.x() * s10);
            s1  = fmadd(w0.x(), s01, w1.x() * s11);
            f0  = fmadd(w0.x(), v00.w(), w1.x() * v10.w());
            f1  = fmadd(w0.x(), v01.w(), w1.x() * v11.w());

            s   = fmadd(w0.y(), s0, w1.y() * s1);
            f   = fmadd(w0.y(), f0, w1.y() * f1);

            /// Evaluate the whitepoint spectrum
            SurfaceInteraction3f si = zero<SurfaceInteraction3f>();
            si.wavelengths = wavelengths;
            UnpolarizedSpectrum wp = m_d65->eval(si, active);

            return s * wp * f * m_scale;
        } else {
            ENOKI_MARK_USED(wavelengths);
            Vector4f v0 = fmadd(w0.x(), v00, w1.x() * v10),
                     v1 = fmadd(w0.x(), v01, w1.x() * v11),
                     v  = fmadd(w0.y(), v0, w1.y() * v1);

            if constexpr (is_monochromatic_v<Spectrum>)
                return head<1>(v) * m_scale;
            else
                return head<3>(v) * m_scale;
        }
    }

    MTS_DECLARE_CLASS()
protected:
        std::string m_filename;
        ScalarBoundingSphere3f m_bsphere;
        ScalarVector2u m_resolution;
        Warp m_warp;
        ref<Texture> m_d65;
        ScalarFloat m_scale;

        ScalarVector2u m_tile_count;
        std::vector<DynamicBuffer<Float>> m_datas;
};

MTS_IMPLEMENT_CLASS_VARIANT(TiledEnvironmentMapEmitter, Emitter)
MTS_EXPORT_PLUGIN(TiledEnvironmentMapEmitter, "TiledEnvironment map emitter")
NAMESPACE_END(mitsuba)
