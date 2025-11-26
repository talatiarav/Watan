#include "boardview.h"

#include <iostream>
#include <sstream>

#include "vertex.h"
#include "edge.h"

using std::string;

std::string BoardView::centre(const std::string &s, int width) {
    if ((int)s.size() >= width) return s;
    int total = width - static_cast<int>(s.size());
    int left  = total / 2;
    int right = total - left;
    return std::string(left, ' ') + s + std::string(right, ' ');
}

BoardView::BoardView(bool enhance,
                     const std::vector<int>       &values,
                     const std::vector<Resources> &resources)
    : enhance{enhance}
{
    // --- init criteria/goals as numbers ---
    criteriaString.resize(54);
    for (int i = 0; i < 54; ++i) {
        if (i < 10) criteriaString[i] = " " + std::to_string(i);
        else        criteriaString[i] = std::to_string(i);
    }

    goalsString.resize(72);
    for (int i = 0; i < 72; ++i) {
        if (i < 10) goalsString[i] = " " + std::to_string(i);
        else        goalsString[i] = std::to_string(i);
    }

    // --- init per-tile strings ---
    resourcesString.resize(19);
    valuesString.resize(19);

    for (int i = 0; i < 19; ++i) {
        // Resource name
        std::string name;
        switch (resources[i]) {
            case Resources::Caffeine: name = "CAFFEINE"; break;
            case Resources::Lab:      name = "LAB";      break;
            case Resources::Lecture:  name = "LECTURE";  break;
            case Resources::Study:    name = "STUDY";    break;
            case Resources::Tutorial: name = "TUTORIAL"; break;
            case Resources::Netflix:  name = "NETFLIX";  break;
            case Resources::None:     name = "";         break;
        }
        resourcesString[i] = name.empty() ? std::string(16, ' ')
                                          : centre(name, 16);

        // Value: blank for Netflix, otherwise centred number
        if (resources[i] == Resources::Netflix || values[i] <= 0) {
            valuesString[i] = std::string(16, ' ');
        } else {
            valuesString[i] = centre(std::to_string(values[i]), 16);
        }
    }
}

void BoardView::notify(Vertex *vertex) {
    if (!vertex) return;

    Colour     c = vertex->getOwnerColour();
    Assessment a = vertex->currentAssessment();
    int        idx = vertex->getId();

    if (idx < 0 || idx >= static_cast<int>(criteriaString.size())) return;

    if (a == Assessment::None) {
        // If "un-built", leave the numeric label
        return;
    }

    string toReplace;

    // Colour code
    if (enhance) {
        if (c == Colour::Blue) {
            toReplace += "\u001b[38;5;33;1mB";
        } else if (c == Colour::Red) {
            toReplace += "\u001b[38;5;196;1mR";
        } else if (c == Colour::Orange) {
            toReplace += "\u001b[38;5;208;1mO";
        } else if (c == Colour::Yellow) {
            toReplace += "\u001b[38;5;11;1mY";
        }
    } else {
        if (c == Colour::Blue)    toReplace += "B";
        else if (c == Colour::Red)    toReplace += "R";
        else if (c == Colour::Orange) toReplace += "O";
        else if (c == Colour::Yellow) toReplace += "Y";
    }

    // Assessment letter
    if (a == Assessment::Assignment) {
        toReplace += enhance ? "A\u001B[0m" : "A";
    } else if (a == Assessment::Midterm) {
        toReplace += enhance ? "M\u001B[0m" : "M";
    } else if (a == Assessment::Exam) {
        toReplace += enhance ? "E\u001B[0m" : "E";
    }

    criteriaString[idx] = toReplace;
}

void BoardView::notify(Edge *edge) {
    if (!edge) return;

    Colour c   = edge->getOwnerColour();
    int    idx = edge->getId();

    if (idx < 0 || idx >= static_cast<int>(goalsString.size())) return;

    // For goals, spec only has Achievement, so we always draw '*A*-like'
    string toReplace;

    if (enhance) {
        if (c == Colour::Blue) {
            toReplace += "\u001b[38;5;33;1mB";
        } else if (c == Colour::Red) {
            toReplace += "\u001b[38;5;196;1mR";
        } else if (c == Colour::Orange) {
            toReplace += "\u001b[38;5;208;1mO";
        } else if (c == Colour::Yellow) {
            toReplace += "\u001b[38;5;11;1mY";
        }
        toReplace += "A\u001B[0m";
    } else {
        if (c == Colour::Blue)    toReplace += "B";
        else if (c == Colour::Red)    toReplace += "R";
        else if (c == Colour::Orange) toReplace += "O";
        else if (c == Colour::Yellow) toReplace += "Y";
        toReplace += "A";
    }

    goalsString[idx] = toReplace;
}

void BoardView::notifyGeese(int tileId) {
    geeseAt = tileId;
}

void BoardView::render(std::ostream &out) const {
    int oddToIncrement = 2;
    int evenToIncrement = 2;
    int oddToRepeat = 0;
    int evenToRepeat = 0;
    int evenGoalPerLine = 2;
    int oddCriterionPerLine = 2;
    int oddGoalPerLine = 1;
    int criteriaLength = criteriaString.size();
    int goalLength = goalsString.size();
    bool odd = true;
    bool decremented = true;
    int lineNum = 1;
    int c = 0; // criterion index
    int g = 0; // goal index
    int v = 0; // value index
    int t = 0; // tile number index
    int r = 0; // resource index

    while (c < criteriaLength && g < goalLength) {
        // Leading spacing by line
        if (lineNum == 1 || lineNum == 2 || lineNum == 40 || lineNum == 41) {
            out << "                                   ";
        } else if (lineNum == 3 || lineNum == 4 ||
                   lineNum == 38 || lineNum == 39) {
            out << "                                 ";
        } else if (lineNum == 5 || lineNum == 6 ||
                   lineNum == 36 || lineNum == 37) {
            out << "                    ";
        } else if (lineNum == 7 || lineNum == 8 ||
                   lineNum == 34 || lineNum == 35) {
            out << "                  ";
        } else if (lineNum == 9 || lineNum == 10 || lineNum == 16 ||
                   lineNum == 17 || lineNum == 18 || lineNum == 24 ||
                   lineNum == 25 || lineNum == 26 || lineNum == 32 ||
                   lineNum == 33) {
            out << "     ";
        } else if (lineNum == 11 || lineNum == 12 || lineNum == 14 ||
                   lineNum == 15 || lineNum == 19 || lineNum == 20 ||
                   lineNum == 22 || lineNum == 23 || lineNum == 27 ||
                   lineNum == 28 || lineNum == 30 || lineNum == 31) {
            out << "   ";
        }

        // Resource / geese lines (hex insides)
        if (lineNum == 2) {
            out << "/            \\" << std::endl;
            ++lineNum;
            continue;
        } else if (lineNum == 4) {
            out << "/" << resourcesString[r] << "\\" << std::endl;
            ++r;
            ++lineNum;
            continue;
        } else if (lineNum == 6) {
            if (geeseAt == 0) {
                out << "/            \\      GEESE     /            \\" << std::endl;
            } else {
                out << "/            \\                /            \\" << std::endl;
            }
            ++lineNum;
            continue;
        } else if (lineNum == 8) {
            out << "/" << resourcesString[r] << "\\";
            ++r;
            out << "            ";
            out << "/" << resourcesString[r] << "\\" << std::endl;
            ++r;
            ++lineNum;
            continue;
        } else if (lineNum == 10 || lineNum == 18 || lineNum == 26) {
            if (lineNum == 10 && geeseAt == 1) {
                out << "/            \\      GEESE     /            \\                /            \\" << std::endl;
            } else if (lineNum == 10 && geeseAt == 2) {
                out << "/            \\                /            \\      GEESE     /            \\" << std::endl;
            } else if (lineNum == 18 && geeseAt == 6) {
                out << "/            \\      GEESE     /            \\                /            \\" << std::endl;
            } else if (lineNum == 18 && geeseAt == 7) {
                out << "/            \\                /            \\      GEESE     /            \\" << std::endl;
            } else if (lineNum == 26 && geeseAt == 11) {
                out << "/            \\      GEESE     /            \\                /            \\" << std::endl;
            } else if (lineNum == 26 && geeseAt == 12) {
                out << "/            \\                /            \\      GEESE     /            \\" << std::endl;
            } else {
                out << "/            \\                /            \\                /            \\" << std::endl;
            }
            ++lineNum;
            continue;
        } else if (lineNum == 12 || lineNum == 20 || lineNum == 28) {
            out << "/" << resourcesString[r] << "\\";
            ++r;
            out << "            ";
            out << "/" << resourcesString[r] << "\\";
            ++r;
            out << "            ";
            out << "/" << resourcesString[r] << "\\" << std::endl;
            ++r;
            ++lineNum;
            continue;
        } else if (lineNum == 14 || lineNum == 22 || lineNum == 30) {
            if ((lineNum == 14 && geeseAt == 3) ||
                (lineNum == 22 && geeseAt == 8) ||
                (lineNum == 30 && geeseAt == 13)) {
                out << "\\      GEESE     /            \\                /            \\                /" << std::endl;
            } else if ((lineNum == 14 && geeseAt == 4) ||
                       (lineNum == 22 && geeseAt == 9) ||
                       (lineNum == 30 && geeseAt == 14)) {
                out << "\\                /            \\      GEESE     /            \\                /" << std::endl;
            } else if ((lineNum == 14 && geeseAt == 5) ||
                       (lineNum == 22 && geeseAt == 10) ||
                       (lineNum == 30 && geeseAt == 15)) {
                out << "\\                /            \\                /            \\      GEESE     /" << std::endl;
            } else {
                out << "\\                /            \\                /            \\                /" << std::endl;
            }
            ++lineNum;
            continue;
        } else if (lineNum == 16 || lineNum == 24 || lineNum == 32) {
            out << "\\            ";
            out << "/" << resourcesString[r] << "\\";
            ++r;
            out << "            ";
            out << "/" << resourcesString[r] << "\\";
            ++r;
            out << "            /" << std::endl;
            ++lineNum;
            continue;
        } else if (lineNum == 34) {
            if (geeseAt == 16) {
                out << "\\      GEESE     /            \\                /" << std::endl;
            } else if (geeseAt == 17) {
                out << "\\                /            \\      GEESE     /" << std::endl;
            } else {
                out << "\\                /            \\                /" << std::endl;
            }
            ++lineNum;
            continue;
        } else if (lineNum == 36) {
            out << "\\            /";
            out << resourcesString[r];
            ++r;
            out << "\\            /" << std::endl;
            ++lineNum;
            continue;
        } else if (lineNum == 38) {
            if (geeseAt == 18) {
                out << "\\      GEESE     /" << std::endl;
            } else {
                out << "\\                /" << std::endl;
            }
            ++lineNum;
            continue;
        } else if (lineNum == 40) {
            out << "\\            /" << std::endl;
            ++lineNum;
            continue;
        }

        int numGoal = 0;

        if (odd) {
            // Odd lines show criteria + goals + values
            if (oddCriterionPerLine == 2 * oddGoalPerLine) {
                // goal between every 2 criteria
                while (numGoal < oddGoalPerLine) {
                    if (numGoal > 0) {
                        out << valuesString[v];
                        ++v;
                    }
                    out << "|" << criteriaString[c] << "|";
                    ++c;
                    out << "--" << goalsString[g] << "--";
                    ++numGoal;
                    out << "|" << criteriaString[c] << "|";
                    ++c;
                }
            } else {
                out << "|" << criteriaString[c] << "|";
                ++c;
                out << valuesString[v];
                ++v;
                while (numGoal < oddGoalPerLine) {
                    if (numGoal > 0) {
                        out << valuesString[v];
                        ++v;
                    }
                    out << "|" << criteriaString[c] << "|";
                    ++c;
                    out << "--" << goalsString[g] << "--";
                    ++numGoal;
                    out << "|" << criteriaString[c] << "|";
                    ++c;
                }
                out << valuesString[v];
                ++v;
                out << "|" << criteriaString[c] << "|";
                ++c;
            }

            if (oddToIncrement > 0) {
                --oddToIncrement;
                oddCriterionPerLine += 2;
                oddGoalPerLine += 1;
            } else if (oddToIncrement == 0) {
                --oddToIncrement;
                oddToRepeat = 6;
                --oddGoalPerLine;
            } else if (oddToRepeat > 0) {
                --oddToRepeat;
                if (decremented) {
                    ++oddGoalPerLine;
                    decremented = false;
                } else {
                    --oddGoalPerLine;
                    decremented = true;
                }
                if (oddToRepeat == 0) {
                    oddCriterionPerLine -= 2;
                }
            } else {
                oddCriterionPerLine -= 2;
                oddGoalPerLine -= 1;
            }
            odd = false;
        } else {
            // Even lines show only goals and tile numbers
            while (numGoal < evenGoalPerLine) {
                if (numGoal > 0) {
                    if (lineNum == 3) {
                        out << tileNumberString[t];
                        ++t;
                    } else if (lineNum == 7) {
                        if (numGoal == 1 || numGoal == 3) {
                            out << tileNumberString[t];
                            ++t;
                        } else {
                            out << "             ";
                        }
                    } else if (lineNum == 11 || lineNum == 19 || lineNum == 27) {
                        if (numGoal == 1 || numGoal == 3 || numGoal == 5) {
                            out << tileNumberString[t];
                            ++t;
                        } else {
                            out << "             ";
                        }
                    } else if (lineNum == 15 || lineNum == 23 || lineNum == 31) {
                        if (numGoal == 2 || numGoal == 4) {
                            out << tileNumberString[t];
                            ++t;
                        } else {
                            out << "             ";
                        }
                    } else if (lineNum == 35) {
                        if (numGoal == 2) {
                            out << tileNumberString[t];
                            ++t;
                        } else {
                            out << "             ";
                        }
                    } else {
                        out << "             ";
                    }
                }
                out << goalsString[g];
                ++numGoal;
                ++g;
            }

            if (evenToIncrement > 0) {
                --evenToIncrement;
                evenGoalPerLine += 2;
            } else if (evenToIncrement == 0) {
                --evenToIncrement;
                evenToRepeat = 4;
            } else if (evenToRepeat > 0) {
                --evenToRepeat;
            } else {
                evenGoalPerLine -= 2;
            }
            odd = true;
        }

        ++lineNum;
        out << std::endl;
    }
}
