#include <Fl/fl_draw.H>
#include <Fl/Fl_Scrollbar.H>
#include "NoteEditor.h"
#include "Viewport.h"

#define SCROLLWIDTH 20

NoteEditor::NoteEditor(int x, int y, int w, int h, Viewport* view) : x(x), y(y), w(w), h(h),
                       view(view), note_thickness(10), ms_per_pixel(10)
{
    scroll_vert = new Fl_Scrollbar(x + w - SCROLLWIDTH - 2, y + 1, SCROLLWIDTH, h - 2);
    scroll_vert->value(40, 30, 0, 127);
}

void NoteEditor::draw() const
{
    int line_y = 0;
    int start_note = scroll_vert->value();

    fl_push_clip(x, y, w, h);
    fl_rectf(x, y, w, h, 0, 0, 0);
    fl_color(150, 150, 150);

    //draw the grey lines separating the notes
    for (int i = start_note; i <= 127; i++){
        if (i % 12 == 1 || i % 12 == 3 || i % 12 == 6 || i % 12 == 8 || i % 12 == 10){
            fl_line(x, y + line_y, x + w, y + line_y);
            line_y += note_thickness;
        } else {
            fl_line(x, y + line_y, x + w, y + line_y);
            line_y += note_thickness + 4;
        }
        drawNoteName(i, x + 4, y + line_y - 2);
        fl_color(150, 150, 150);
    }

    drawNotes();
    fl_color(0, 50, 200);
    fl_line(x + 300, y, x + 300, y + h);
    scroll_vert->redraw();
    fl_pop_clip();
}

void NoteEditor::move(int x, int y)
{
    this->x = x;
    this->y = y;
    scroll_vert->resize(x + w - SCROLLWIDTH - 2, y + 1, SCROLLWIDTH, h - 2);
}

void NoteEditor::resize(int w, int h)
{
    this->w = w;
    this->h = h;
    scroll_vert->resize(x + w - SCROLLWIDTH - 2, y + 1, SCROLLWIDTH, h - 2);
}

void NoteEditor::setThickness(int thickness)
{
    note_thickness = thickness;
    view->redraw();
}

void NoteEditor::setMsPerPixel(int ms)
{
    ms_per_pixel = ms;
}

int NoteEditor::getMsPerPixel() const
{
    return ms_per_pixel;
}

void NoteEditor::getNotePos(int note_value, unsigned long time, int &x, int &y) const
{
    int start_note = scroll_vert->value();
    x = this->x + ((signed long)time - (signed long)view->getPlayback()->getTime()) / ms_per_pixel + 300;

    if (note_value < start_note){
        y = this->y - 20; //outside clip area, so invisible
        return;
    }
    y = this->y;
    for (int i = start_note; i < note_value; i++) {
        if (isBlackNote(i)) {
            y += note_thickness;
        } else {
            y += note_thickness + 4;
        }
    }
}

int NoteEditor::getNoteThickness(int note_value) const
{
    if (isBlackNote(note_value)){
        return note_thickness;
    } else {
        return note_thickness + 4;
    }
}

bool NoteEditor::isBlackNote(int i) const
{
    return (i % 12 == 1 || i % 12 == 3 || i % 12 == 6 || i % 12 == 8 || i % 12 == 10);
}

void NoteEditor::drawNotes() const
{
    long draw_from = view->getPlayback()->getTime() - 300 * ms_per_pixel;
    int num_tracks = view->getMIDIData()->numTracks();
    int num_events;
    Track* track;
    char r, g, b;

    for (int i = 0; i < num_tracks; i++){
        track = view->getMIDIData()->getTrack(i);
        num_events = track->numEvents();
        track->getColour(r, g, b);
        fl_color(r, g, b);

        for (int idx = 0; idx < num_events; idx++){
            //stop when when the notes are off-screen
            if ((signed long)(track->getEvent(idx)->getTime()) >
                   draw_from + this->w * ms_per_pixel){
                break;
            }
            if ((signed long)(track->getEvent(idx)->getTime()) >= draw_from ||
                    (signed long)(track->getEvent(idx)->getTime()) +
                    track->getEvent(idx)->getDuration() >= draw_from){
                track->getEvent(idx)->draw();
            }
        }
    }
}

void NoteEditor::drawNoteName(int note, int x, int y) const
{
    int value = note % 12;
    fl_color(50, 230, 50);
    fl_font(FL_COURIER|FL_BOLD|FL_ITALIC, 15);

    switch(value){
        case 0:
            if (note == 60){ //middle C
                fl_color(200, 50, 50);
            } else {
                fl_color(0, 255, 255); //every other C is cyan
            }
            fl_draw("C", x, y);
            break;
        case 2:
            fl_draw("D", x, y);
            break;
        case 4:
            fl_draw("E", x, y);
            break;
        case 5:
            fl_draw("F", x, y);
            break;
        case 7:
            fl_draw("G", x, y);
            break;
        case 9:
            fl_draw("A", x, y);
            break;
        case 11:
            fl_draw("B", x, y);
            break;
        default:
            break;
    }
}
