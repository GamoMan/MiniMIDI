/*  MiniMIDI: A simple, lightweight, crossplatform MIDI editor.
 *  Copyright (C) 2016 Nicholas Parkanyi
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <Fl/fl_draw.H>
#include "MIDI.h"
#include "Viewport.h"

NoteOn::NoteOn(Viewport* view, Track* track, unsigned long time, short channel,
               short value, short velocity, int duration)
               : ChannelEvent(view, track, "NoteOn", time, channel),
                 value(value), velocity(velocity), duration(duration)
{}

short NoteOn::getValue() const
{
    return value;
}

int NoteOn::getDuration() const
{
    return duration;
}

void NoteOn::setDuration(int duration)
{
    this->duration = duration;
}

void NoteOn::run()
{
    char r, g, b;
    track->getColour(r, g, b);
    view->getPlayback()->getSynth()->noteOn(getChannel(), value, velocity);
    view->getKeyboard()->setKey(value, true, r, g, b);
    view->redraw();
}

void NoteOn::draw()
{
    int x, y;
    NoteEditor* editor = view->getEditor();
    int w = duration / editor->getMsPerPixel();
    int h = editor->getNoteThickness(value);

    editor->getNotePos(value, getTime(), x, y);
    fl_rectf(x, y + 1, w, h - 1);
}

NoteOff::NoteOff(Viewport* view, Track* track, unsigned long time, short channel,
                 short value)
                 : ChannelEvent(view, track, "NoteOff", time, channel), value(value)
{}

short NoteOff::getValue() const
{
    return value;
}

void NoteOff::run()
{
    view->getPlayback()->getSynth()->noteOff(getChannel(), value);
    view->getKeyboard()->setKey(value, false, 0, 0, 0);
    view->redraw();
}

void NoteOff::draw()
{}

Track::Track() : r(255), g(255), b(255)
{
    //events.reserve();
}

ProgramChange::ProgramChange(Viewport* view, Track* track, unsigned long time,
                             short channel, short voice)
                             : ChannelEvent(view, track, "ProgramChange", time, channel), voice(voice)
{}

short ProgramChange::getVoice() const
{
    return voice;
}

void ProgramChange::run()
{
    view->getPlayback()->getSynth()->programChange(getChannel(), voice);
}

void ProgramChange::draw()
{}

unsigned long Track::getDuration() const
{
    int n = events.size();
    if (n == 0){
        return 0;
    } else {
        return events[n - 1]->getTime();
    }
}

void Track::addEvent(std::shared_ptr<Event> ev)
{
    //index of first event occurring at the same time or after the event
    //we are inserting
    int idx = getEventAt(ev->getTime());
    if (idx >= 0){
        events.insert(events.begin() + idx, ev);
    } else {
        events.push_back(ev);
    }
}

void Track::appendEvent(std::shared_ptr<Event> ev)
{
    events.push_back(ev);
}

void Track::removeEvent(std::shared_ptr<Event> ev)
{
    int size = events.size();
    for (int i = 0; i < size; i++){
        if (events[i] == ev){
            events.erase(events.begin() + i);
            break;
        }
    }
}

void Track::removeNotesAt(unsigned long time, int value)
{
    int trk_size = numEvents();

    //find NoteOn events that are occurring at time on this note
    for (int idx = 0; idx < trk_size; idx++){
        if (getEvent(idx)->getTime() <= time &&
                time <= getEvent(idx)->getTime() + getEvent(idx)->getDuration() &&
                getEvent(idx)->getType() == std::string("NoteOn") &&
                static_cast<NoteOn*>(getEvent(idx).get())->getValue() == value){
            //find the associated NoteOff event
            for (int i = idx; i < trk_size; i++){
                if (getEvent(i)->getType() == std::string("NoteOff") &&
                        static_cast<NoteOff*>(getEvent(i).get())->getValue() == value){
                    removeEvent(getEvent(idx));
                    removeEvent(getEvent(--i)); //decr. i because we've removed an event,
                    return;
                }
            }
        }
    }
}

int Track::numEvents() const
{
    return events.size();
}

std::shared_ptr<Event> Track::getEvent(int index) const
{
    return events[index];
}

int Track::getEventAt(long time) const
{
    if (time < 0){
        return 0;
    }

    int size = events.size();
    int i = size / 2;
    int jump = i / 2;
    if (jump == 0) { jump = 1; }

    //if no event occurs at or after the given time, return -1
    if (events.empty() || events[size - 1]->getTime() < time){
        return -1;
    }

    //find first event that occurs at or after the given time
    while (i >= 0 && i <= size - 1 &&
           !((i == 0 || events[i-1]->getTime() < time)
             && events[i]->getTime() >= time)){
        jump /= 2;
        if (jump == 0) { jump = 1; }
        if (events[i]->getTime() < time){
            i += jump;
        } else {
            i -= jump;
        }
    }

    return i;
}

void Track::setColour(char r, char g, char b)
{
    this->r = r;
    this->g = g;
    this->b = b;
}

void Track::getColour(char &r, char &g, char &b) const
{
    r = this->r;
    g = this->g;
    b = this->b;
}

Playback::Playback(Viewport* view) : view(view), time_elapsed(0), playing(false)
{}

unsigned long Playback::getTime() const
{
    if (playing){
        return std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::steady_clock::now() - start_time).count();
    } else {
        return time_elapsed;
    }
}

Synth* Playback::getSynth()
{
    return &synth;
}

void Playback::seek(unsigned long time)
{
    MIDIData* data = view->getMIDIData();
    Track* track;
    int num_tracks = data->numTracks();

    time_elapsed = time;
    updateIndices();
    for (int i = 0; i < num_tracks; i++){
        track = data->getTrack(i);
        track_indices[i] = track->getEventAt(time);
        if (track_indices[i] == -1){
            track_indices[i] = track->numEvents(); //the track is over,
                                                   //set index to last event + 1
        }
    }
    view->getKeyboard()->clear();
    view->redraw();
    synth.clear();

    if (playing)
        play();
}

void Playback::pause()
{
    time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
                 (std::chrono::steady_clock::now() - start_time).count();
    playing = false;
}

void Playback::play()
{
    MIDIData* data = view->getMIDIData();
    std::chrono::milliseconds ms(time_elapsed);
    start_time = std::chrono::steady_clock::now() - ms;
    time_elapsed = 0;
    for (int i = 0; i < data->numTracks(); i++){
        updateIndices();
    }
    playing = true;
}

void Playback::updateIndices()
{
    int new_tracks = view->getMIDIData()->numTracks() - track_indices.size();
    for (int i = 0; i < new_tracks; i++){
        track_indices.push_back(0);
    }
}

void Playback::everyFrame()
{
    MIDIData* data = view->getMIDIData();
    int num_events;
    int num_tracks = data->numTracks();
    Track* track;
    if (playing){
        for (int i = 0; i < num_tracks; i++){
            track = data->getTrack(i);
            num_events = track->numEvents();
            while (track_indices[i] < num_events &&
                   track->getEvent(track_indices[i])->getTime() <= getTime()){
                track->getEvent(track_indices[i])->run();
                track_indices[i]++;
            }
        }
    }
}

std::string Playback::getTimeString() const
{
    unsigned long time = getTime();
    std::ostringstream tstr;
    tstr << time / 60000 << ":" << std::setfill('0') << std::setw(2)
        << (time % 60000) / 1000;
    return tstr.str();
}

MIDIData::MIDIData(Viewport* view) : view(view), filename("")
{}

void MIDIData::fillTrack()
{
}

int MIDIData::numTracks() const
{
    return tracks.size();
}

Track* MIDIData::getTrack(int index)
{
    return &tracks[index];
}

void MIDIData::newTrack()
{
	//red, blue, light green, orange, cyan, dark green, yellow, pink
	int r_bank[] = {241, 15, 15, 255, 15, 15, 255, 241};
	int g_bank[] = {25, 91, 255, 78, 255, 147, 255, 25};
	int b_bank[] = {10, 255, 15, 15, 255, 15, 15, 196};
	static int idx = 0;



    tracks.push_back(Track());
    tracks[tracks.size() - 1].setColour(r_bank[idx], g_bank[idx], b_bank[idx]);

	//generate new colour for next track
	idx = (idx + 1) % 8;
}

void MIDIData::clear()
{
    tracks.clear();
}
