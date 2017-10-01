/*********************************************************************
(c) Matt Marchant 2017
http://trederia.blogspot.com

xygineXT - Zlib license.

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

#include "InventoryDirector.hpp"
#include "MessageIDs.hpp"
#include "PacketIDs.hpp"
#include "MapData.hpp"

#include <xyginext/network/NetHost.hpp>
#include <xyginext/ecs/Scene.hpp>

#include <limits>

namespace
{
    const sf::Uint32 NpcScore = 500;
    const sf::Uint32 GooblyScore = 850;
    const sf::Uint32 SmallFruitScore = 350;
    const sf::Uint32 LargeFruitScore = 1000;

    const sf::Uint32 lifeScore = 10000; //extra life is awarded in multiples of this
    const sf::Uint8 maxLives = 5;
}

InventoryDirector::InventoryDirector(xy::NetHost& host)
    : m_host(host)
{

}

//public
void InventoryDirector::handleMessage(const xy::Message& msg)
{    
    //used to check if we got an extra life
    auto p1Old = m_playerValues[0].score / lifeScore;
    auto p2Old = m_playerValues[1].score / lifeScore;
    
    switch (msg.id)
    {
    default: break;
    case MessageID::NpcMessage:
    {
        const auto& data = msg.getData<NpcEvent>();
        if (data.type == NpcEvent::Died)
        {
            auto actor = getScene().getEntity(data.entityID).getComponent<Actor>().type;
            
            if (actor == ActorID::Goobly)
            {
                m_playerValues[data.playerID].score += GooblyScore;
                sendUpdate(data.playerID, GooblyScore);
            }
            else
            {
                m_playerValues[data.playerID].score += NpcScore;
                sendUpdate(data.playerID, NpcScore);
            }            
        }
    }
        break;
    case MessageID::ItemMessage:
    {
        sf::Uint32 amount = 0;
        const auto& data = msg.getData<ItemEvent>();
        switch (data.actorID)
        {
        default: break;
        case ActorID::FruitLarge:

            break;
        case ActorID::FruitSmall:
            amount = SmallFruitScore;
            break;
        }

        m_playerValues[data.playerID].score += amount;
        sendUpdate(data.playerID, amount);
    }
        break;
    case MessageID::PlayerMessage:
    {
        const auto& data = msg.getData<PlayerEvent>();
        if (data.type == PlayerEvent::Spawned)
        {
            //reset the old scores
            const auto& player = data.entity.getComponent<Player>();
            m_playerValues[player.playerNumber].lives = player.lives;
            m_playerValues[player.playerNumber].score = 0;

            //send info on both, in case new player has joined
            sendUpdate(0, 0);
            sendUpdate(1, 0);
        }
        else if (data.type == PlayerEvent::Died)
        {
            const auto& player = data.entity.getComponent<Player>();
            m_playerValues[player.playerNumber].lives = player.lives;
            sendUpdate(player.playerNumber, 0);
        }
    }
        break;
    case MessageID::NetworkMessage:
    {
        const auto& data = msg.getData<NetworkEvent>();
        if (data.type == NetworkEvent::Disconnected)
        {
            m_playerValues[data.playerID].lives = 0;
            m_playerValues[data.playerID].score = 0;
            sendUpdate(data.playerID, 0);
        }
    }
        break;
    }

    checkLifeBonus(0, p1Old);
    checkLifeBonus(1, p2Old);
}

//private
void InventoryDirector::sendUpdate(sf::Uint8 player, sf::Uint32 amount)
{
        InventoryUpdate update;
        update.lives = m_playerValues[player].lives;
        update.score = m_playerValues[player].score;
        update.amount = amount;
        update.playerID = player;

        m_host.broadcastPacket(PacketID::InventoryUpdate, update, xy::NetFlag::Reliable, 1);
}

void InventoryDirector::checkLifeBonus(sf::Uint8 player, sf::Uint32 oldScore)
{
    auto newScore = m_playerValues[player].score / lifeScore;
    if (newScore > oldScore && m_playerValues[player].lives < maxLives)
    {
        m_playerValues[player].lives++;
        sendUpdate(player, std::numeric_limits<sf::Uint32>::max()); //flag to say this is a life update rather than score
    }
}