export module Assessment;

// Single source of truth for “what’s built” at a location.
// - On Vertex: Assignment / Midterm / Exam
// - On Edge: Achievement
// - Or None (unbuilt)
export enum class Assessment {
    Achievement,
    Assignment,
    Midterm,
    Exam,
    None
};
