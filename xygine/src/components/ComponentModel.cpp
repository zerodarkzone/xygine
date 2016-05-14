/*********************************************************************
Matt Marchant 2014 - 2016
http://trederia.blogspot.com

xygine - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <xygine/components/Model.hpp>
#include <xygine/Entity.hpp>

#include <glm/gtc/matrix_transform.hpp>

using namespace xy;

Model::Model(MessageBus& mb, const Mesh& mesh, const MeshRenderer::Lock&)
    : Component     (mb, this),
    m_mesh          (mesh),
    m_depth         (0.f)
{

}

void Model::entityUpdate(Entity& entity, float dt)
{
    const auto position = entity.getWorldPosition();
    m_worldMatrix = glm::translate(glm::mat4(), glm::vec3(position.x, position.y, m_depth));
}

//private
void Model::draw(const glm::mat4& viewMatrix) const
{

}