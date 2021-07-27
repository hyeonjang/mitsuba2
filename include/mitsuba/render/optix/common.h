#pragma once

#ifdef __CUDACC__
# include <optix.h>
#endif

#include <mitsuba/render/optix/bbox.cuh>
#include <mitsuba/render/optix/matrix.cuh>

struct OptixHitGroupData {
    unsigned long long shape_ptr;
    void* data;
};

/// Launch-varying parameters
struct OptixParams {
    bool    *in_mask;
    float   *in_o[3],
            *in_d[3],
            *in_mint, *in_maxt;
    float   *out_t, 
            *out_uv[2],
            *out_ng[3],
            *out_ns[3],
            *out_p[3],
            *out_dp_du[3],
            *out_dp_dv[3];

    unsigned long long *out_shape_ptr;
    unsigned int *out_prim_index;

    bool *out_hit;

    OptixTraversableHandle handle;
    bool fill_surface_interaction;
};

#ifdef __CUDACC__
extern "C" {
    __constant__ OptixParams params;
}

using namespace optix;

__device__ void write_output_params(OptixParams &params,
                                    unsigned int launch_index,
                                    unsigned long long shape_ptr,
                                    unsigned int prim_id,
                                    const Vector3f &p,
                                    const Vector2f &uv,
                                    const Vector3f &ns,
                                    const Vector3f &ng,
                                    const Vector3f &dp_du,
                                    const Vector3f &dp_dv,
                                    float t) {

    params.out_shape_ptr[launch_index] = shape_ptr;
    params.out_prim_index[launch_index] = prim_id;

    params.out_uv[0][launch_index] = uv.x();
    params.out_uv[1][launch_index] = uv.y();

    params.out_p[0][launch_index] = p.x();
    params.out_p[1][launch_index] = p.y();
    params.out_p[2][launch_index] = p.z();

    if (params.fill_surface_interaction) {
        params.out_ng[0][launch_index] = ng.x();
        params.out_ng[1][launch_index] = ng.y();
        params.out_ng[2][launch_index] = ng.z();

        params.out_ns[0][launch_index] = ns.x();
        params.out_ns[1][launch_index] = ns.y();
        params.out_ns[2][launch_index] = ns.z();


        params.out_dp_du[0][launch_index] = dp_du.x();
        params.out_dp_du[1][launch_index] = dp_du.y();
        params.out_dp_du[2][launch_index] = dp_du.z();

        params.out_dp_dv[0][launch_index] = dp_dv.x();
        params.out_dp_dv[1][launch_index] = dp_dv.y();
        params.out_dp_dv[2][launch_index] = dp_dv.z();
    }

    params.out_t[launch_index] = t;
}

/// Returns a unique launch index per ray
__device__ unsigned int calculate_launch_index() {
    uint3 launch_dims = optixGetLaunchDimensions();
    uint3 launch_index3 = optixGetLaunchIndex();
    return launch_index3.x + (launch_index3.y + launch_index3.z * launch_dims.y) * launch_dims.x;
}
#endif