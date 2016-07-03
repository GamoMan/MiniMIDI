#ifndef SYNTH_H
#define SYNTH_H
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
#include <string>
#include "fluidsynth.h"
 
 class Synth {
 public:
    Synth(std::string driver, std::string sf2_file);
    ~Synth();
    
    //whether fluidsynth was successfully loaded, if it fails, playback will be silent
    bool initialized();
    void reload(std::string driver, std::string sf_file);
    std::string getDriver();
    std::string getSF();
    void noteOn(short value, int velocity);
    void noteOff(short value);
    
private:
    bool is_initialized;
    std::string driver;
    std::string sf_file;
    fluid_settings_t* settings;
    fluid_synth_t* synth;
    fluid_audio_driver_t* adriver;
    int sf_handle;
}
 
#endif /* SYNTH_H */