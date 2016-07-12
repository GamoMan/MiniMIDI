#ifndef NOTEEDITOR_H
#define NOTEEDITOR_H

class Viewport;

class NoteEditor {
public:
    NoteEditor(int x, int y, int w, int h, Viewport* view);

    void draw() const;
    void move(int x, int y);
    void resize(int w, int h);
    //sets the top note drawn on screen, by its MIDI value (0-127).
    void scroll(int note_value);
    //sets the thickness of the drawn black notes
    void setThickness(int thickness);
    //sets the number of milliseconds per pixel
    void setMsPerPixel(int ms);
    int getMsPerPixel() const;
    //returns absolute y position of this note on the NoteEditor
    void getNotePos(int note_value, unsigned long time, int &x, int &y) const;
    //returns the thickness of this note
    int getNoteThickness(int note_value) const;

private:
    bool isBlackNote(int note_value) const;
    void drawNotes() const;

    int x, y, w, h;
    Viewport* view;
    int start_note;
    int note_thickness;
    int ms_per_pixel;
};

#endif // NOTEEDITOR_H
