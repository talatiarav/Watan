module Player;

import <algorithm>;
import <cstdlib>;
import <ctime>;
import <iostream>;
import <sstream>;
import <map>;
import <random>;
import <vector>;

import Dice;
import Vertex;
import Edge;
import Assessment;
import Resources;
import Fair;
import Loaded;
import Colour;

using std::cout;
using std::endl;
using std::string;

namespace {

// Order in save file: Caff, Lab, Lect, Study, Tut
int getResourceCount(const std::map<Resources,int> &res, Resources r) {
    auto it = res.find(r);
    return (it == res.end() ? 0 : it->second);
}

// Map Assessment → save level (1/2/3)
int assessmentLevel(Assessment a) {
    switch (a) {
        case Assessment::Assignment: return 1;
        case Assessment::Midterm:    return 2;
        case Assessment::Exam:       return 3;
        default:                     return 0; // None = no building
    }
}

} // namespace


// --- ctor ---

Player::Player(Colour colour)
    : colour{colour} {}

// --- dice ---

void Player::setDice(std::unique_ptr<Dice> newDice) {
    dice = std::move(newDice);
}

// --- resources / ownership ---

void Player::addResources(Resources resource, int amount) {
    resources[resource] += amount;
    if (resources[resource] <= 0) {
        resources.erase(resource);
    }
}

int Player::numResources() const {
    int total = 0;
    for (auto const &entry : resources) {
        total += entry.second;
    }
    return total;
}

void Player::addVertex(Vertex *v) {
    if (!v) return;
    ownedVertices.emplace_back(v);
}

void Player::addEdge(Edge *e) {
    if (!e) return;
    ownedEdges.emplace_back(e);
}

// --- saving ---

std::string Player::encodeResourcesForSave() const {
    auto getCount = [this](Resources r) {
        auto it = resources.find(r);
        if (it == resources.end()) return 0;
        return it->second;
    };

    int numCaffeines = getCount(Resources::Caffeine);
    int numLabs      = getCount(Resources::Lab);
    int numLectures  = getCount(Resources::Lecture);
    int numStudies   = getCount(Resources::Study);
    int numTutorials = getCount(Resources::Tutorial);

    std::ostringstream oss;
    oss << numCaffeines << ' '
        << numLabs      << ' '
        << numLectures  << ' '
        << numStudies   << ' '
        << numTutorials;

    return oss.str();
}

std::string Player::encodeGoalsForSave() const {
    std::ostringstream oss;
    bool first = true;
    for (Edge *e : ownedEdges) {
        if (!e) continue;
        if (!first) oss << ' ';
        first = false;
        oss << e->getId(); // assumes Edge has getId()
    }
    return oss.str();
}

std::string Player::encodeVerticesForSave() const {
    std::ostringstream oss;
    bool first = true;
    for (Vertex *v : ownedVertices) {
        if (!v) continue;
        Assessment a = v->currentAssessment();
        int level = 0;
        if (a == Assessment::Assignment)      level = 1;
        else if (a == Assessment::Midterm)    level = 2;
        else if (a == Assessment::Exam)       level = 3;
        else                                  continue; // skip None / Achievement on vertices

        if (!first) oss << ' ';
        first = false;
        oss << v->getId() << ' ' << level; // assumes Vertex has getId()
    }
    return oss.str();
}

std::string Player::encodeForSave() const {
    std::ostringstream oss;

    // 1. Resources in order: Caff, Lab, Lect, Study, Tut
    int caff   = ::getResourceCount(resources, Resources::Caffeine);
    int lab    = ::getResourceCount(resources, Resources::Lab);
    int lect   = ::getResourceCount(resources, Resources::Lecture);
    int study  = ::getResourceCount(resources, Resources::Study);
    int tut    = ::getResourceCount(resources, Resources::Tutorial);

    oss << caff  << ' '
        << lab   << ' '
        << lect  << ' '
        << study << ' '
        << tut   << ' ';

    // 2. Edges (roads) after 'g'
    oss << 'g';
    for (Edge *e : ownedEdges) {
        // Edge needs: int getId() const;
        oss << ' ' << e->getId();
    }

    // 3. Vertices (criteria) after 'c'
    oss << " c";
    for (Vertex *v : ownedVertices) {
        // Vertex needs: int getId() const; Assessment currentAssessment() const;
        Assessment a = v->currentAssessment();
        int level = assessmentLevel(a);
        if (level == 0) continue; // skip if it's still None / unbuilt

        oss << ' ' << v->getId() << ' ' << level;
    }

    return oss.str();
}


// --- points / rules ---

int Player::getPoints() const {
    int points = 0;
    for (Vertex *v : ownedVertices) {
        if (!v) continue;
        Assessment a = v->currentAssessment();
        if (a == Assessment::Assignment)      points += 1;
        else if (a == Assessment::Midterm)    points += 2;
        else if (a == Assessment::Exam)       points += 3;
    }
    return points;
}

bool Player::resourcesCheck(Assessment type) const {
    auto get = [this](Resources r) {
        auto it = resources.find(r);
        if (it == resources.end()) return 0;
        return it->second;
    };

    switch (type) {
        case Assessment::Assignment:
            if (get(Resources::Caffeine) < 1) return false;
            if (get(Resources::Lab)      < 1) return false;
            if (get(Resources::Lecture)  < 1) return false;
            if (get(Resources::Tutorial) < 1) return false;
            return true;

        case Assessment::Midterm:
            if (get(Resources::Lecture) < 2) return false;
            if (get(Resources::Study)   < 3) return false;
            return true;

        case Assessment::Exam:
            if (get(Resources::Caffeine) < 3) return false;
            if (get(Resources::Lab)      < 2) return false;
            if (get(Resources::Lecture)  < 2) return false;
            if (get(Resources::Tutorial) < 1) return false;
            if (get(Resources::Study)    < 2) return false;
            return true;

        case Assessment::Achievement:
            if (get(Resources::Tutorial) < 1) return false;
            if (get(Resources::Study)    < 1) return false;
            return true;

        case Assessment::None:
        default:
            return true;
    }
}

void Player::resourcesSpent(Assessment type) {
    switch (type) {
        case Assessment::Assignment:
            addResources(Resources::Caffeine, -1);
            addResources(Resources::Lab,      -1);
            addResources(Resources::Lecture,  -1);
            addResources(Resources::Tutorial, -1);
            break;

        case Assessment::Midterm:
            addResources(Resources::Lecture, -2);
            addResources(Resources::Study,   -3);
            break;

        case Assessment::Exam:
            addResources(Resources::Caffeine, -3);
            addResources(Resources::Lab,      -2);
            addResources(Resources::Lecture,  -2);
            addResources(Resources::Tutorial, -1);
            addResources(Resources::Study,    -2);
            break;

        case Assessment::Achievement:
            addResources(Resources::Tutorial, -1);
            addResources(Resources::Study,    -1);
            break;

        case Assessment::None:
        default:
            break;
    }
}

// Geese logic (port of Student::loseResources with cleaned-up total count).
void Player::loseResourcesToGeese(std::ostream &out) {
    int total = numResources();
    if (total < 10) return;

    int numLost = total / 2;

    out << "Student " << colour << " loses " << numLost
        << " resources to the geese. They lose:" << endl;

    // Collect keys (resource types) that we might lose.
    std::vector<Resources> keys;
    keys.reserve(resources.size());
    for (auto const &entry : resources) {
        if (entry.second > 0) {
            keys.emplace_back(entry.first);
        }
    }

    int lostCaffeine  = 0;
    int lostLab       = 0;
    int lostLecture   = 0;
    int lostStudy     = 0;
    int lostTutorial  = 0;

    if (keys.empty()) return;

    // Use std::rand() to seed the mt19937 engine
    // This way it respects the global seed set by std::srand() in main
    std::mt19937 gen(std::rand());

    for (int i = 0; i < numLost; ++i) {
        std::shuffle(keys.begin(), keys.end(), gen);
        // find a resource type that is still available
        while (!keys.empty() && resources[keys.front()] == 0) {
            std::shuffle(keys.begin(), keys.end(), gen);
        }
        Resources r = keys.front();
        resources[r] -= 1;

        switch (r) {
            case Resources::Caffeine: ++lostCaffeine; break;
            case Resources::Lab:      ++lostLab;      break;
            case Resources::Lecture:  ++lostLecture;  break;
            case Resources::Study:    ++lostStudy;    break;
            case Resources::Tutorial: ++lostTutorial; break;
            default: break;
        }
    }

    if (lostCaffeine  > 0) out << lostCaffeine  << " Caffeine"  << endl;
    if (lostLab       > 0) out << lostLab       << " Lab"       << endl;
    if (lostLecture   > 0) out << lostLecture   << " Lecture"   << endl;
    if (lostStudy     > 0) out << lostStudy     << " Study"     << endl;
    if (lostTutorial  > 0) out << lostTutorial  << " Tutorial"  << endl;

    // Clean up any zero entries.
    for (auto it = resources.begin(); it != resources.end(); ) {
        if (it->second <= 0) it = resources.erase(it);
        else ++it;
    }
}

void Player::reset() {
    resources.clear();
    ownedVertices.clear();
    ownedEdges.clear();
    dice.reset();
}

// --- printing / status ---

std::string Player::formatResourcesStatus() const {
    auto get = [this](Resources r) {
        auto it = resources.find(r);
        if (it == resources.end()) return 0;
        return it->second;
    };

    int numCaffeines = get(Resources::Caffeine);
    int numLabs      = get(Resources::Lab);
    int numLectures  = get(Resources::Lecture);
    int numStudies   = get(Resources::Study);
    int numTutorials = get(Resources::Tutorial);

    std::ostringstream oss;
    oss << numCaffeines << " caffeines, "
        << numLabs      << " labs, "
        << numLectures  << " lectures, "
        << numStudies   << " studies, and "
        << numTutorials << " tutorials";

    return oss.str();
}

void Player::printStatus(std::ostream &out) const {
    out << colour << " has "
        << getPoints() << " course criteria, "
        << formatResourcesStatus()
        << endl;
}

void Player::printCriteria(std::ostream &out) const {
    out << colour << " has completed:" << endl;
    for (Vertex *v : ownedVertices) {
        if (!v) continue;
        Assessment a = v->currentAssessment();
        int level = 0;
        if (a == Assessment::Assignment)      level = 1;
        else if (a == Assessment::Midterm)    level = 2;
        else if (a == Assessment::Exam)       level = 3;
        else                                  continue;

        out << v->getId() << ' ' << level << endl;
    }
}


void Player::setResourcesFromSave(int caffeines,
                                  int labs,
                                  int lectures,
                                  int studies,
                                  int tutorials) {
    resources.clear();

    if (caffeines > 0) {
        resources[Resources::Caffeine] = caffeines;
    }
    if (labs > 0) {
        resources[Resources::Lab] = labs;
    }
    if (lectures > 0) {
        resources[Resources::Lecture] = lectures;
    }
    if (studies > 0) {
        resources[Resources::Study] = studies;
    }
    if (tutorials > 0) {
        resources[Resources::Tutorial] = tutorials;
    }
    // Netflix is never stored as a "card" in the save format.
}

int Player::getResourceCount(Resources r) const {
    auto it = resources.find(r);
    return (it == resources.end() ? 0 : it->second);
}

void Player::addResource(Resources r, int amount) {
    if (amount <= 0) return;
    resources[r] += amount;
}

void Player::removeResource(Resources r, int amount) {
    if (amount <= 0) return;
    auto it = resources.find(r);
    if (it == resources.end()) return;
    if (it->second <= amount) {
        resources.erase(it);
    } else {
        it->second -= amount;
    }
}

// dice methods
void Player::useFairDice() {
    dice = std::make_unique<Fair>();
}

void Player::useLoadedDice() {
    dice = std::make_unique<Loaded>();
}

bool Player::isLoadedDice() const {
    // dynamic_cast is fine here; only two concrete types.
    return dynamic_cast<Loaded *>(dice.get()) != nullptr;
}

void Player::setLoadedRoll(int value) {
    if (auto *ld = dynamic_cast<Loaded *>(dice.get())) {
        ld->setDie(value);
    }
}

int Player::rollDice() {
    if (!dice) {
        // default to fair if somehow not set
        dice = std::make_unique<Fair>();
    }
    return dice->roll();
}
