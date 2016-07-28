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

#include <CommandIds.hpp>
#include <TilemapDemoState.hpp>

#include <xygine/Reports.hpp>
#include <xygine/Entity.hpp>

#include <xygine/App.hpp>
#include <xygine/Log.hpp>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
    const sf::Keyboard::Key upKey = sf::Keyboard::W;
    const sf::Keyboard::Key downKey = sf::Keyboard::S;
    const sf::Keyboard::Key leftKey = sf::Keyboard::A;
    const sf::Keyboard::Key rightKey = sf::Keyboard::D;
    const sf::Keyboard::Key fireKey = sf::Keyboard::Space;

    const float joyDeadZone = 25.f;
    const float joyMaxAxis = 100.f;
}

TilemapDemoState::TilemapDemoState(xy::StateStack& stateStack, Context context)
    : State     (stateStack, context),
    m_messageBus(context.appInstance.getMessageBus()),
    m_scene     (m_messageBus),
    m_physWorld (m_messageBus)
{
    launchLoadingScreen();

    buildScene();

    quitLoadingScreen();
}

bool TilemapDemoState::update(float dt)
{    
    m_scene.update(dt);
    return true;
}

void TilemapDemoState::draw()
{
    auto& rw = getContext().renderWindow;
    rw.draw(m_scene);
    rw.setView(getContext().defaultView);
    /*if (drawOverlay)*/ rw.draw(m_physWorld);
}

bool TilemapDemoState::handleEvent(const sf::Event& evt)
{   
    switch (evt.type)
    {
    case sf::Event::MouseButtonReleased:
    {
        //const auto& rw = getContext().renderWindow;
        //auto mousePos = rw.mapPixelToCoords(sf::Mouse::getPosition(rw));

    }
    break;
    case sf::Event::KeyPressed:
        switch (evt.key.code)
        {
        case upKey:

            break;
        case downKey:

            break;
        case leftKey:

            break;
        case rightKey:

            break;
        case fireKey:

            break;
        default: break;
        }
        break;
    case sf::Event::KeyReleased:
        switch (evt.key.code)
        {
        case sf::Keyboard::Escape:
        case sf::Keyboard::BackSpace:
            requestStackPop();
            requestStackPush(States::ID::MenuMain);
            break;
        case sf::Keyboard::P:
            //requestStackPush(States::ID::MenuPaused);
            break;
        case upKey:

            break;
        case downKey:

            break;
        case leftKey:

            break;
        case rightKey:

            break;
        case fireKey:

            break;
        default: break;
        }
        break;
    case sf::Event::JoystickButtonPressed:
        switch (evt.joystickButton.button)
        {
        case 0:

            break;
        }
        break;
    case sf::Event::JoystickButtonReleased:

        switch (evt.joystickButton.button)
        {
        case 0:

            break;
        case 7:

            break;
        default: break;
        }
        break;
    }

    return true;
}

void TilemapDemoState::handleMessage(const xy::Message& msg)
{
    m_scene.handleMessage(msg);
}

//private
void TilemapDemoState::buildScene()
{
    if (m_tilemap.load("assets/maps/platform.tmx"))
    {
        auto entity = xy::Entity::create(m_messageBus);

        const auto& layers = m_tilemap.getLayers();
        for (const auto& l : layers)
        {
            if (l->getType() == xy::tmx::Layer::Type::Object)
            {
                xy::Logger::log("found object layer - attempting to create physics components", xy::Logger::Type::Info);
                auto rb = m_tilemap.createRigidBody(m_messageBus, *dynamic_cast<xy::tmx::ObjectGroup*>(l.get()));
                entity->addComponent(rb);
            }
        }

        m_scene.addEntity(entity, xy::Scene::Layer::BackFront);
    }
}