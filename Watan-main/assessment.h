export module Assessment;

// Represents the type of completion on a course criterion (or none).
// Used to track whether a criterion is an Assignment, Midterm, Exam,
// and to determine upgrade order and resource payouts.

export enum class Assessment {
    Achievement,
    Assignment,
    Midterm,
    Exam,
    None
};

