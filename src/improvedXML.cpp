#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <cctype>
#include <algorithm>

class MyXML {
public:
    enum class State {
        START,
        BEGIN_TAG,
        TAG,
        END_TAG,
        TERM_TAG,
        END_TERM_TAG,
        EOF_STATE
    };

    explicit MyXML(std::ifstream& input) : m_fileInput(&input), m_stringInputPos(0), m_state(State::START), m_nesting(0) {}
    explicit MyXML(const std::string& input) : m_stringInput(input), m_fileInput(nullptr), m_stringInputPos(0), m_state(State::START), m_nesting(0) {}
    
    ~MyXML() = default;

    // Delete copy constructor and assignment operator
    MyXML(const MyXML&) = delete;
    MyXML& operator=(const MyXML&) = delete;

    // Move constructor and assignment operator
    MyXML(MyXML&&) noexcept = default;
    MyXML& operator=(MyXML&&) noexcept = default;

    void resetParser() {
        m_buffer.clear();
        m_state = State::START;
        m_nesting = 0;
        m_stringInputPos = 0;
        if (m_fileInput) {
            m_fileInput->clear();
            m_fileInput->seekg(0);
        }
    }

    void rewind() {
        resetParser();
    }

    std::string parse() {
        m_buffer.clear();
        int c;
        while ((c = nextChar()) != EOF) {
            switch (m_state) {
                case State::START:
                    if (c == '<') m_state = State::BEGIN_TAG;
                    break;
                case State::BEGIN_TAG:
                    if (std::isspace(c)) break;
                    if (c == '/') {
                        m_state = State::TERM_TAG;
                    } else if (c == '>') {
                        m_state = State::END_TAG;
                    } else {
                        m_state = State::TAG;
                        m_buffer.push_back(static_cast<char>(c));
                    }
                    break;
                case State::TAG:
                    if (c == '>') {
                        m_state = State::END_TAG;
                        m_nesting++;
                        return m_buffer;
                    } else if (!std::isspace(c)) {
                        m_buffer.push_back(static_cast<char>(c));
                    }
                    break;
                case State::END_TAG:
                    if (c == '<') {
                        m_state = State::BEGIN_TAG;
                        return m_buffer;
                    } else {
                        m_buffer.push_back(static_cast<char>(c));
                    }
                    break;
                case State::TERM_TAG:
                    if (c == '>') {
                        m_state = State::END_TERM_TAG;
                        m_nesting--;
                        return m_buffer;
                    } else if (!std::isspace(c)) {
                        m_buffer.push_back(static_cast<char>(c));
                    }
                    break;
                case State::END_TERM_TAG:
                    if (c == '<') m_state = State::BEGIN_TAG;
                    break;
                default:
                    throw std::runtime_error("Invalid parser state");
            }
        }
        m_state = State::EOF_STATE;
        return {};
    }

    int getNesting() const { return m_nesting; }

    std::string getNextTag() {
        std::string result;
        do {
            result = parse();
        } while (m_state != State::END_TAG && !result.empty());
        return result;
    }

    std::string getNextDataField() {
        std::string result;
        do {
            result = parse();
        } while (m_state != State::BEGIN_TAG && !result.empty());
        return result;
    }

    std::string getNextTaggedField(const std::string& tag) {
        std::string result = getNextTag(tag);
        if (m_state == State::END_TAG && !result.empty()) {
            result = getNextDataField();
        }
        return result;
    }

    std::string getNextTag(const std::string& tag) {
        std::string result;
        while (result != tag) {
            result = getNextTag();
            if (result.empty()) break;
        }
        return result;
    }

private:
    std::string m_buffer;
    State m_state;
    int m_nesting;
    std::ifstream* m_fileInput;
    std::string m_stringInput;
    size_t m_stringInputPos;

    int nextChar() {
        if (m_fileInput) {
            return m_fileInput->get();
        } else if (m_stringInputPos < m_stringInput.length()) {
            return m_stringInput[m_stringInputPos++];
        }
        return EOF;
    }
};