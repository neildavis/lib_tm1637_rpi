#include "tm1637_sayer.h"
#include "tm1637_digits.h"
#include "tm1637.h"

using namespace tm1637;

Sayer::Sayer(std::unique_ptr<Device> &device, const std::string &str) 
    : m_device(std::move(device)) {

    for (auto it=str.begin(); it != str.end(); it++) {
        switch (*it) {
        case '0':           m_digits.push_back(DIGIT_0);                break;
        case '1':           m_digits.push_back(DIGIT_1);                break;
        case '2':           m_digits.push_back(DIGIT_2);                break;
        case '3':           m_digits.push_back(DIGIT_3);                break;
        case '4':           m_digits.push_back(DIGIT_4);                break;
        case '5':           m_digits.push_back(DIGIT_5);                break;
        case '6':           m_digits.push_back(DIGIT_6);                break;
        case '7':           m_digits.push_back(DIGIT_7);                break;
        case '8':           m_digits.push_back(DIGIT_8);                break;
        case '9':           m_digits.push_back(DIGIT_9);                break;
        //              
        case 'a':           m_digits.push_back(DIGIT_a);                break;
        case 'A':           m_digits.push_back(DIGIT_A);                break;
        case 'b':           m_digits.push_back(DIGIT_b);                break;
        case 'B':           m_digits.push_back(DIGIT_B);                break;
        case 'c':           m_digits.push_back(DIGIT_c);                break;
        case 'C':           m_digits.push_back(DIGIT_C);                break;
        case 'd': case 'D': m_digits.push_back(DIGIT_d);                break;
        case 'e':           m_digits.push_back(DIGIT_e);                break;
        case 'E':           m_digits.push_back(DIGIT_E);                break;
        case 'f': case 'F': m_digits.push_back(DIGIT_F);                break;
        case 'g':           m_digits.push_back(DIGIT_g);                break;
        case 'G':           m_digits.push_back(DIGIT_G);                break;
        case 'h':           m_digits.push_back(DIGIT_h);                break;
        case 'H':           m_digits.push_back(DIGIT_H);                break;
        case 'i': case 'I': m_digits.push_back(DIGIT_I);                break;
        case 'j': case 'J': m_digits.push_back(DIGIT_J);                break;
        case 'l': case 'L': m_digits.push_back(DIGIT_L);                break;
        case 'm':
        case 'M':
            m_digits.push_back(DIGIT_M_L);
            m_digits.push_back(DIGIT_M_R);
            break;
        case 'n':           m_digits.push_back(DIGIT_n);                break;
        case 'N':           m_digits.push_back(DIGIT_N);                break;
        case 'o':           m_digits.push_back(DIGIT_o);                break;
        case 'O':           m_digits.push_back(DIGIT_O);                break;
        case 'p': case 'P': m_digits.push_back(DIGIT_P);                break;
        case 'q': case 'Q': m_digits.push_back(DIGIT_q);                break;
        case 'r': case 'R': m_digits.push_back(DIGIT_r);                break;
        case 's': case 'S': m_digits.push_back(DIGIT_S);                break;
        case 't': case 'T': m_digits.push_back(DIGIT_t);                break;
        case 'u':           m_digits.push_back(DIGIT_u);                break;
        case 'U':           m_digits.push_back(DIGIT_U);                break;
        case 'w':
        case 'W':
            m_digits.push_back(DIGIT_W_L);
            m_digits.push_back(DIGIT_W_R);
            break;
        case 'y': case 'Y': m_digits.push_back(DIGIT_y);                break;
        case 'z': case 'Z': m_digits.push_back(DIGIT_Z);                break;
        //
        case '-':           m_digits.push_back(DIGIT_HYPHEN);           break;
        case '_':           m_digits.push_back(DIGIT_UNDERSCORE);       break;
        case ' ':           m_digits.push_back(DIGIT_SPACE);    break;
        //
        case '?':
        default:            m_digits.push_back(DIGIT_QUESTION_MARK);            break; 
        }
    }
    // Push four spaces at end to end scrolling with last letter on LHS
    for (int i = 0; i < 4; i++) {
        m_digits.push_back(DIGIT_SPACE);
    }
}

void Sayer::begin() {
    m_device->clear();
    m_data[0] = 0;
    m_data[1] = 0;
    m_data[2] = 0;
    m_data[3] = 0;
    m_it = m_digits.begin();
}

bool Sayer::next() {
    if (m_it == m_digits.end()) {
        return false;
    }
    // Shift down existing data
    m_data[0] = m_data[1];
    m_data[1] = m_data[2];
    m_data[2] = m_data[3];
    // Grab new digit
    m_data[3] = *m_it;
    // Show on device
    m_device->showRawDigits(m_data);
    m_it++;
    return true;
}

void Sayer::clear() {
    m_device->clear();
}
