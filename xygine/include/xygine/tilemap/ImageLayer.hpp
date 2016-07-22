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

#ifndef XY_TMX_IMAGELAYER_HPP_
#define XY_TMX_IMAGELAYER_HPP_

#include <xygine/tilemap/Layer.hpp>

#include <SFML/Graphics/Texture.hpp>

namespace xy
{
    class TextureResource;
    namespace tmx
    {
        class XY_EXPORT_API ImageLayer final : public Layer
        {
        public:
            ImageLayer(const std::string&, xy::TextureResource&);
            ~ImageLayer() = default;

            Type getType() const override { return Layer::Type::Image; }
            void parse(const pugi::xml_node&) override;

            const sf::Texture& getTexture() const { return m_texture; }

        private:
            sf::Texture m_texture;
            std::string m_workingDir;
            xy::TextureResource& m_textureResource;
        };
    }
}
#endif //XY_TMX_IMAGELAYER_HPP_