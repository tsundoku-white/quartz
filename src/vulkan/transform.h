#pragma once 
#include "../core/core.h"
#include <cstdint>
#include <glm/fwd.hpp>
#include <strings.h>

struct TransformHandle
{
  uint32_t index = UINT32_MAX;
  [[nodiscard]] bool is_valid() const { return index != UINT32_MAX; }
};

struct TransformPool
{
  std::vector<glm::vec3> location;
  std::vector<glm::quat> rotation;
  std::vector<glm::vec3> scale;

  std::vector<glm::vec3> right;
  std::vector<glm::quat> forward;
  std::vector<glm::vec3> up;

  std::vector<bool> dirty;
  uint32_t count    = 0;
};

namespace transform_system
{
  TransformHandle create(TransformPool &pool);
  void destroy(TransformPool& pool, TransformHandle handle);
  void destroy_all(TransformPool &pool);
  void update_all(TransformPool &pool);

  void set_location(TransformPool& pool, TransformHandle handle, glm::vec3 value);
  void set_rotation(TransformPool& pool, TransformHandle handle, glm::quat value);
  void set_scale   (TransformPool& pool, TransformHandle handle, glm::vec3 value);
  void set_f_rotation(TransformPool& pool, TransformHandle handle, float x, float y, float z);



  [[nodiscard]] inline glm::vec3 get_location(const TransformPool &pool, TransformHandle handle)
  {
    return pool.location[handle.index];
  }
  [[nodiscard]] inline glm::quat get_rotation(const TransformPool &pool, TransformHandle handle)
  {
    return pool.rotation[handle.index];
  }
  [[nodiscard]] inline glm::vec3 get_scale(const TransformPool &pool, TransformHandle handle)
  {
    return pool.scale[handle.index];
  }

  [[nodiscard]] inline glm::vec3 get_right(const TransformPool &pool, TransformHandle handle)
  {
    return pool.location[handle.index]; 
  }
  [[nodiscard]] inline glm::quat get_forward(const TransformPool &pool, TransformHandle handle)
  {
    return pool.rotation[handle.index];
  }
  [[nodiscard]] inline glm::vec3 get_up(const TransformPool &pool, TransformHandle handle)
  {
    return pool.scale[handle.index]; 
  }
}

