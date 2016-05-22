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

#ifndef XY_MESH_RENDERER_HPP_
#define XY_MESH_RENDERER_HPP_

#include <xygine/mesh/Material.hpp>
#include <xygine/mesh/UniformBuffer.hpp>
#include <xygine/MultiRenderTexture.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

namespace xy
{
    class MessageBus;
    class Message;
    class Mesh;
    class Model;
    class Scene;
    /*!
    \brief Mesh rendering class. The MeshRenderer is
    responsible for drawing the model components in a scene.
    The MeshRenderer automatically translates the given scene's camera
    into 3D space and aligns it with the 2D world so that 3D models will
    fit, creating a 2.5D like effect. The mesh renderer itself is an
    SFML drawable, so can be drawn on top of a scene like any other
    drawable class.
    */
    class XY_EXPORT_API MeshRenderer final : public sf::Drawable
    {
        friend class Model;
    public:
        /*!
        \brief Constructor.
        \param sf::Vector2u Size of the scene to render. By default xygine
        uses xy::DefaultSceneSize but if rendering the scene with multiple
        cameras it is necessary to set the size to match the camera view.
        \param Scene An instance of the scene containing the model components
        which belong to this MeshRenderer
        */
        MeshRenderer(const sf::Vector2u&, const Scene&);
        ~MeshRenderer() = default;

        /*!
        \brief Factory function for creating Model components.
        Model components require registration with the MeshRenderer
        so the only valid way to create them is via this factory function.
        */
        std::unique_ptr<Model> createModel(MessageBus&, const Mesh&);

        /*!
        \brief Updates the MeshRenderer with the current scene status.
        Should be called once per frame after updating the scene to which
        the MeshRenderer is associated.
        */
        void update();

        /*!
        \brief Message Handler.
        System messages should be passed to this handler once per frame.
        */
        void handleMessage(const Message&);

        /*!
        \brief Returns the UniformBuffer containing the MeshRenderer's
        matrix uniforms.
        In order to render properly a shader's material requires the renderer's
        projection and view matrix. This UniformBuffer will automatically update
        the properties of any material which uses a shader with a u_matrixBlock
        uniform.
        \see Shader3D::DefaultVertex
        */
        const UniformBuffer& getMatrixUniforms() const { return m_matrixBlockBuffer; }

        /*!
        \brief Returns the UniformBuffer containing the MeshRenderer's lighting
        properties.
        The MeshRenderer updates the lighting properties from its associated scene
        and scene lighting components. This properties are stored in this uniform
        buffer and updated automatically, and therefore should be used by any material
        which requires scene lighting information.
        */
        const UniformBuffer& getLightingUniforms() const { return m_lightingBlockBuffer; }

    private:
        struct Lock final {};

        sf::Shader m_defaultShader;
        std::unique_ptr<Material> m_defaultMaterial;

        const Scene& m_scene;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
        float m_cameraZ;

        struct Point final
        {
            float position[3];
            float padding;
        };

        struct MatrixBlock final
        {
            //use raw arrays 'cos GLSL
            float u_viewMatrix[16];
            float u_projectionMatrix[16];
            //group these here too as we're all in the vertex shader
            Point u_pointLightPositions[8];
            float u_cameraWorldPosition[3];
        }m_matrixBlock;
        UniformBuffer m_matrixBlockBuffer;

        struct PointLight final
        {
            float diffuseColour[4];
            float specularColour[4];
            float inverseRange;
            float intensity;
            float padding[2]; //GLSL must align to multiple of vec4          
        };
        struct LightBlock final
        {
            PointLight u_pointLights[8];
        }m_lightingBlock;
        UniformBuffer m_lightingBlockBuffer;

        std::array<sf::Glsl::Vec3, 128> m_ssaoKernel;
        mutable sf::Shader m_ssaoShader;
        mutable sf::RenderTexture m_ssaoTexture;
        sf::Sprite m_ssaoSprite;

        mutable std::vector<Model*> m_models;
        mutable xy::MultiRenderTexture m_renderTexture;
        sf::Sprite m_sprite;
        void drawScene() const;

        void draw(sf::RenderTarget&, sf::RenderStates) const override;

        void updateView();
        void updateLights(const glm::vec3&);
    };

}

#endif //XY_MESH_RENDERER_HPP_