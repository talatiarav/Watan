#ifndef ASSESSMENT_H
#define ASSESSMENT_H

// Single source of truth for “what’s built” at a location.
// - On Vertex: Assignment / Midterm / Exam
// - On Edge: Achievement
// - Or None (unbuilt)
enum class Assessment {
    Achievement,
    Assignment,
    Midterm,
    Exam,
    None
};

#endif
