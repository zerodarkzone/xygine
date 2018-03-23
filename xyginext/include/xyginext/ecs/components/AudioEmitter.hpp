/*********************************************************************
(c) Matt Marchant 2017 - 2018
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

#ifndef XY_AUDIO_EMITTER_HPP_
#define XY_AUDIO_EMITTER_HPP_

#include "xyginext/Config.hpp"
#include "xyginext/audio/AudioSourceImpl.hpp"

#include <SFML/Config.hpp>

#include <memory>
#include <string>

namespace sf
{
    class SoundBuffer;
}

namespace xy
{
    namespace detail
    {
        class AudioSourceImpl;
    }

    /*!
    \brief Audio emitter component.
    Audio emitter provide points within the scene to play
    audio, either one shot effects or streaming music. If the
    buffer used to supply the audio is mono, and the scene contains
    an AudioSystem, then the AudioEmitter will be panned spatially
    relative to the current listener (usually the active scene camera),
    unless it has been set to absolute positioning.
    */
    class XY_EXPORT_API AudioEmitter final
    {
    public:
        enum Status
        {
            Stopped = 0,
            Paused,
            Playing
        };

        /*!
        \brief Constructor.
        When AudioEmitters are first created they are in an invalid
        state, and will not play any sound until setSource() has
        been called to assign a valid sound source.
        */
        AudioEmitter();

        /*!
        \brief Sets the source of the emitter to a buffer
        shared between one or more emitters to play sounds
        fully loaded in memory. This is usually applied to
        short sounds used as sfx. This will remove any existing
        audio source, including any which are loaded as streaming
        sources.
        */
        void setSource(const sf::SoundBuffer&);

        /*!
        \brief Sets the source of the emitter to stream a file
        at a given path.
        Usually longer audio such as music should be streamed
        rather than loading it entirely into memory. This will
        remove any existing sound source, even if loading fails.
        \returns true if the source was opened successfully, else false
        */
        bool setSource(const std::string& path);

        /*!
        \brief Returns true if the emitter has been initialised with a sound source
        */
        bool hasSource() const;

        /*!
        \brief Plays the emitter's sound source if there is one
        */
        void play();

        /*!
        \brief Pauses any currently playing audio.
        */
        void pause();

        /*!
        \brief Stops any playing audio, and rewinds the audio to
        the beginning if it is playing or paused.
        */
        void stop();

        /*
        \brief Sets the playback pitch of the emitter.
        A value of one is normal speed, 0 is effectively stopped
        and 2 is twice the speed etc.
        */
        void setPitch(float);

        /*
        \brief Set the playback volume of this emitter.
        The volume is multiplied by the master volume as well
        as the channel volume assigned to this emitter.
        The default volume is 1, and 0 is effectively muted
        \see setChannel()
        */
        void setVolume(float);

        /*!
        \brief Set the 2D position of the emitter in world
        coordinates. This is automatically overriden is the
        entity to which this emitter is attached also has a
        Transform component and exists in a scene that contains
        an AudioSystem. This also has no effect if the source
        for this component is stereo
        */
        void setPosition(sf::Vector2f);

        /*!
        \brief Sets whether or not this sound is panned relatively
        to the active listener. Does not affect stereo sound sources.
        */
        void setRelativeTolistener(bool);

        /*!
        \brief Sets the minimum distance at which the emitter
        starts to fade, when relative to the active listener
        */
        void setMinDistance(float);

        /*!
        \brief Sets the rolloff, ie how quickly the sound
        fades with distance from the listener.
        */
        void setAttenuation(float);

        /*!
        \brief Sets whether or not this sound should be played looped.
        */
        void setLooped(bool);

        /*!
        \brief Sets the channel in the Audio mixer.
        Emitters can be assigned mixer channels which in turn affect
        the volume of all sounds assigned to the channel. For example
        you may wish to assign all sound effects to channel 2, and UI
        sounds to channel 3. This way sound effects can have their volume
        adjusted independently oif UI effects. By default all emitters
        are assigned to channel 0
        \see Mixer
        */
        void setChannel(sf::Uint8);

        /*!
        \brief Returns the current playback pitch of the emitter
        */
        float getPitch() const;

        /*!
        \brief Returns the current playback volume of the emitter
        */
        float getVolume() const;

        /*!
        \brief Returns the current 2D position of the emitter in 
        world coordinates'
        This only applies to mono sounds which are not automatically
        panned by their entitiy's transform component.
        */
        sf::Vector2f getPosition() const;

        /*!
        \brief Returns whether or not this emitter is set tot be panned
        relatively to the active listener
        */
        bool isRelativeToListener() const;

        /*!
        \brief Returns the minimum distance from the listener at which
        the emitter's volume starts to fade.
        */
        float getMinDistance() const;

        /*!
        \brief Returns the current attentuaton value
        */
        float getAttenuation() const;

        /*!
        \brief Returns whether or not this emitter's sound is set to
        play looped.
        */
        bool isLooped() const;

        /*!
        \brief Returns the current mixer channel assignment
        */
        sf::Uint8 getChannel() const { return m_mixerChannel; }

        /*!
        \brief Returns the current status of the sound.
        Can be Playing, Paused or Stopped.
        */
        Status getStatus() const;

    private:
        sf::Uint8 m_mixerChannel;
        float m_volume;
        std::unique_ptr<Detail::AudioSourceImpl> m_impl;

        friend class AudioSystem;
    };
}

#endif //XY_AUDIO_EMITTER_HPP_
