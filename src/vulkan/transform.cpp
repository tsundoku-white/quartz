#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

TransformHandle transform_system::create(TransformPool& pool)
{
    TransformHandle handle{};
    handle.index = pool.count;

    pool.location.push_back(glm::vec3(0.0f));
    pool.rotation.push_back(glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
    pool.scale.push_back(glm::vec3(1.0f));

    pool.right.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    pool.forward.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
    pool.up.push_back(glm::vec3(0.0f, 1.0f, 0.0f));

    pool.dirty.push_back(true);

    pool.count++;
    return handle;
}

void transform_system::destroy(TransformPool& pool, TransformHandle handle)
{
    if (!handle.is_valid() || handle.index >= pool.count)
        return;

    uint32_t last = pool.count - 1;

    pool.location[handle.index] = pool.location[last];
    pool.rotation[handle.index] = pool.rotation[last];
    pool.scale[handle.index]    = pool.scale[last];

    pool.right[handle.index]    = pool.right[last];
    pool.forward[handle.index]  = pool.forward[last];
    pool.up[handle.index]       = pool.up[last];

    pool.dirty[handle.index]    = pool.dirty[last];

    pool.location.pop_back();
    pool.rotation.pop_back();
    pool.scale.pop_back();

    pool.right.pop_back();
    pool.forward.pop_back();
    pool.up.pop_back();

    pool.dirty.pop_back();

    pool.count--;
}

void transform_system::destroy_all(TransformPool& pool)
{
    pool.location.clear();
    pool.rotation.clear();
    pool.scale.clear();

    pool.right.clear();
    pool.forward.clear();
    pool.up.clear();

    pool.dirty.clear();

    pool.count = 0;
}

void transform_system::set_location(TransformPool& pool, TransformHandle handle, glm::vec3 value)
{
    if (!handle.is_valid() || handle.index >= pool.count)
        return;

    pool.location[handle.index] = value;
    pool.dirty[handle.index]    = true;
}

void transform_system::set_rotation(TransformPool& pool, TransformHandle handle, glm::quat value)
{
    if (!handle.is_valid() || handle.index >= pool.count)
        return;

    pool.rotation[handle.index] = value;
    pool.dirty[handle.index]    = true;
}

void transform_system::set_scale(TransformPool& pool, TransformHandle handle, glm::vec3 value)
{
    if (!handle.is_valid() || handle.index >= pool.count)
        return;

    pool.scale[handle.index] = value;
    pool.dirty[handle.index] = true;
}

void transform_system::update_all(TransformPool& pool)
{
    for (uint32_t i = 0; i < pool.count; ++i)
    {
        if (!pool.dirty[i])
            continue;

        glm::mat4 rot_mat = glm::toMat4(pool.rotation[i]);

        pool.right[i]   = glm::normalize(glm::vec3(rot_mat[0][0], rot_mat[0][1], rot_mat[0][2]));
        pool.up[i]      = glm::normalize(glm::vec3(rot_mat[1][0], rot_mat[1][1], rot_mat[1][2]));
        pool.forward[i] = -glm::normalize(glm::vec3(rot_mat[2][0], rot_mat[2][1], rot_mat[2][2]));

        pool.dirty[i] = false;
    }
}
