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

#include <xygine/mesh/MeshRenderer.hpp>
#include <xygine/mesh/StaticConsts.hpp>
#include <xygine/components/Model.hpp>
#include <xygine/components/PointLight.hpp>
#include <xygine/Scene.hpp>
#include <xygine/util/Const.hpp>
#include <xygine/Reports.hpp>

#include <SFML/Graphics/RenderStates.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstring>

using namespace xy;

namespace
{
    const float fov = 30.f * xy::Util::Const::degToRad;
    const float nearPlane = 0.1f; 
}

MeshRenderer::MeshRenderer(const sf::Vector2u& size, const Scene& scene)
    : m_scene               (scene),
    m_matrixBlockBuffer     ("u_matrixBlock"),
    m_lightingBlockBuffer   ("u_lightBlock")
{
    //set up a default material to assign to newly created models
    m_defaultShader.loadFromMemory(Shader3D::DefaultVertex, Shader3D::DefaultFragment);
    m_defaultMaterial = std::make_unique<Material>(m_defaultShader);
    
    //create the render buffer
    m_renderTexture.create(size.x, size.y, 1u, true);
    m_sprite.setTexture(m_renderTexture.getTexture(0));

    updateView();

    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16 * sizeof(float));
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.create(m_matrixBlock);

    m_defaultMaterial->addUniformBuffer(m_matrixBlockBuffer);
    
    //set lighting buffer
    m_lightingBlockBuffer.create(m_lightingBlock);
    m_defaultMaterial->addUniformBuffer(m_lightingBlockBuffer);
}

//public
std::unique_ptr<Model> MeshRenderer::createModel(MessageBus& mb, const Mesh& mesh)
{
    auto model = Component::create<Model>(mb, mesh, Lock());
    m_models.push_back(model.get());
    
    //set default material
    model->setBaseMaterial(*m_defaultMaterial);

    return std::move(model);
}

void MeshRenderer::update()
{
    m_models.erase(std::remove_if(m_models.begin(), m_models.end(),
        [](const Model* m)
    {
        return m->destroyed();
    }), m_models.end());

    auto view = m_scene.getView();
    auto camPos = view.getCenter();
    auto rotation = view.getRotation() * xy::Util::Const::degToRad;
    glm::vec3 camWorldPosition(camPos.x, camPos.y, m_cameraZ);

    m_viewMatrix = glm::translate(glm::mat4(), camWorldPosition);
    //rotate
    m_viewMatrix = glm::rotate(m_viewMatrix, rotation, glm::vec3(0.f, 0.f, 1.f));
    m_viewMatrix = glm::inverse(m_viewMatrix);
    
    std::memcpy(m_matrixBlock.u_viewMatrix, glm::value_ptr(m_viewMatrix), 16 * sizeof(float));
    m_matrixBlockBuffer.update(m_matrixBlock);

    //update UBO with scene lighting / cam pos
    updateLights(camWorldPosition);
}

void MeshRenderer::handleMessage(const Message& msg)
{
    //handle camera changed event
    if (msg.id == xy::Message::SceneMessage)
    {
        const auto& msgData = msg.getData<xy::Message::SceneEvent>();
        switch (msgData.action)
        {
        default:
        case xy::Message::SceneEvent::CameraChanged:
            updateView();
            break;
        }
    }
}

//private
void MeshRenderer::drawScene() const
{    
    m_renderTexture.setActive(true);
    
    auto viewPort = m_renderTexture.getView().getViewport();
    glViewport(
        static_cast<GLuint>(viewPort.left * xy::DefaultSceneSize.x), 
        static_cast<GLuint>(viewPort.top * xy::DefaultSceneSize.y),
        static_cast<GLuint>(viewPort.width * xy::DefaultSceneSize.x),
        static_cast<GLuint>(viewPort.height * xy::DefaultSceneSize.y));

    glClearColor(1.f, 1.f, 1.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE); //TODO apply these in material passes
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    for (const auto& m : m_models)m->draw(m_viewMatrix);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    m_renderTexture.display();
    //m_renderTexture.setActive(false);
}

void MeshRenderer::draw(sf::RenderTarget& rt, sf::RenderStates states) const
{
    drawScene();
    rt.draw(m_sprite, states);
}

void MeshRenderer::updateView()
{
    auto viewSize = m_scene.getView().getSize();

    //calc how far away from the scene the camera would be
    //assuming the 2D world is drawn at 0 depth
    const float angle = std::tan(fov / 2.f);
    m_cameraZ = ((viewSize.y / 2.f) / angle);

    m_projectionMatrix = glm::perspective(fov, viewSize.x / viewSize.y, nearPlane, m_cameraZ * 2.f);
    std::memcpy(m_matrixBlock.u_projectionMatrix, glm::value_ptr(m_projectionMatrix), 16);

    //XY_WARNING(std::abs(m_cameraZ) > farPlane, "Camera depth greater than far plane: " + std::to_string(m_cameraZ));
}

void MeshRenderer::updateLights(const glm::vec3& camWorldPosition)
{
    m_lightingBlock.u_cameraWorldPosition[0] = camWorldPosition.x;
    m_lightingBlock.u_cameraWorldPosition[1] = camWorldPosition.y;
    m_lightingBlock.u_cameraWorldPosition[2] = camWorldPosition.z;

    //update active lights
    const auto lights = m_scene.getVisibleLights(m_scene.getVisibleArea());
    auto i = 0;
    for (const auto& light : lights)
    {
        auto& colour = light->getDiffuseColour();
        m_lightingBlock.u_pointLights[i].diffuseColour[0] = static_cast<float>(colour.r) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[1] = static_cast<float>(colour.g) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[2] = static_cast<float>(colour.b) / 255.f;
        m_lightingBlock.u_pointLights[i].diffuseColour[3] = static_cast<float>(colour.a) / 255.f;

        auto& spec = light->getSpecularColour();
        m_lightingBlock.u_pointLights[i].specularColour[0] = static_cast<float>(spec.r) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[1] = static_cast<float>(spec.g) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[2] = static_cast<float>(spec.b) / 255.f;
        m_lightingBlock.u_pointLights[i].specularColour[3] = static_cast<float>(spec.a) / 255.f;

        m_lightingBlock.u_pointLights[i].intensity = light->getIntensity();
        m_lightingBlock.u_pointLights[i].inverseRange = light->getInverseRange();

        auto& position = light->getWorldPosition();
        m_lightingBlock.u_pointLights[i].position[0] = position.x;
        m_lightingBlock.u_pointLights[i].position[1] = position.y;
        m_lightingBlock.u_pointLights[i].position[2] = position.z;

        i++;
    }

    //turn off others by setting intensity to 0
    while (i++ < Shader3D::MAX_LIGHTS)
    {
        m_lightingBlock.u_pointLights[i].intensity = 0.f;
    }

    m_lightingBlockBuffer.update(m_lightingBlock);
}