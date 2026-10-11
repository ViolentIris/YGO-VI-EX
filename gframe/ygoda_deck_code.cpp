#include "ygoda_deck_code.h"
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <sstream>
#include <bitset>
#include <cstring>
#include <cwchar>

bool is_base64(char c) {
    return (isalnum(c) || (c == '+') || (c == '/') || (c == '='));
}

// Base64 encoding and decoding functions with URL-safe variant
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i < 4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}

std::vector<unsigned char> base64_decode(std::string const& encoded_string) {
    size_t in_len = encoded_string.size();
    int i = 0;
    size_t in = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in] != '=') && is_base64(encoded_string[in])) {
        char_array_4[i++] = encoded_string[in]; in++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = static_cast<unsigned char>(base64_chars.find(char_array_4[i]));

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (int j = 0; j < 4; j++)
            char_array_4[j] = static_cast<unsigned char>(base64_chars.find(char_array_4[j]));

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

// URL-safe Base64 functions
std::string b64url_encode(const std::vector<unsigned char>& bytes) {
    std::string b64 = base64_encode(bytes.data(), bytes.size());
    // Replace + with -, / with _, and remove =
    for (auto& c : b64) {
        if (c == '+') c = '-';
        else if (c == '/') c = '_';
    }
    // Remove padding '='
    b64.erase(std::remove(b64.begin(), b64.end(), '='), b64.end());
    return b64;
}

std::vector<unsigned char> b64url_decode(std::string s) {
    // Replace - with +, _ with /
    for (auto& c : s) {
        if (c == '-') c = '+';
        else if (c == '_') c = '/';
    }
    // Add padding
    int pad_len = (4 - (s.length() % 4)) % 4;
    for (int i = 0; i < pad_len; i++) {
        s += '=';
    }
    return base64_decode(s);
}

// Helper function to convert integer to binary string with fixed width
std::string int_to_binary(int value, int width) {
    std::string result = "";
    for (int i = width - 1; i >= 0; i--) {
        result += ((value >> i) & 1) ? '1' : '0';
    }
    return result;
}

// Helper function to parse a bit string to integer
int binary_to_int(const std::string& bits, int start, int length) {
    int result = 0;
    for (int i = 0; i < length; i++) {
        if (start + i < bits.length() && bits[start + i] == '1') {
            result |= (1 << (length - 1 - i));
        }
    }
    return result;
}

// Find next group of same cards (up to 3)
std::vector<int> next_group(const std::vector<int>& ids, int i) {
    if (i >= ids.size()) {
        return std::vector<int>{0, 0, i};
    }
    
    int cid = ids[i];
    int n = 1;
    int j = i + 1;
    
    // Count up to 3 cards of the same ID
    while (j < ids.size() && ids[j] == cid && n < 3) {
        n++;
        j++;
    }
    
    return std::vector<int>{cid, n, j};
}

// Encode bits according to the protocol
std::pair<std::string, std::vector<int>> encode_bits(const std::vector<int>& main, 
                                                     const std::vector<int>& extra, 
                                                     const std::vector<int>& side) {
    std::vector<std::vector<int>> groups = {main, extra, side};
    std::vector<int> counts;
    std::string body = "";
    
    for (const auto& g : groups) {
        // Count unique cards (groups)
        int cnt = 0;
        int i = 0;
        
        // keep original order (no sorting) - protocol encodes as-is
        std::vector<int> temp_g = g;
        
        while (i < temp_g.size()) {
            if (temp_g[i] > 0) {
                std::vector<int> result = next_group(temp_g, i);
                int cid = result[0];
                int n = result[1];
                int next_i = result[2];
                if (cid > 0) {
                    cnt++;
                    // Add quantity prefix (2 bits) + card ID (27 bits)
                    std::string prefix = (n == 1) ? "01" : (n == 2) ? "10" : "11";
                    body += prefix + int_to_binary(cid, 27);
                }
                i = next_i;
            } else {
                i++;
            }
        }
        counts.push_back(cnt);
    }
    
    // Header: main count (8 bits) + extra count (4 bits) + side count (4 bits)
    std::string hdr = int_to_binary(counts[0], 8) + 
                      int_to_binary(counts[1], 4) + 
                      int_to_binary(counts[2], 4);
    
    return std::make_pair(hdr + body, counts);
}

// Convert bit string to bytes
std::vector<unsigned char> to_bytes(const std::string& bits) {
    std::vector<unsigned char> result;
    
    // Pad with zeros if length is not multiple of 8
    std::string padded_bits = bits;
    if (padded_bits.length() % 8 != 0) {
        int padding = 8 - (padded_bits.length() % 8);
        padded_bits += std::string(padding, '0');
    }
    
    // Convert every 8 bits to a byte
    for (size_t i = 0; i < padded_bits.length(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++) {
            if (i + j < padded_bits.length() && padded_bits[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        result.push_back(byte);
    }
    
    return result;
}

// Parse v1 bit string
std::vector<std::vector<int>> _parse_v1_bits(const std::string& bits) {
    if (bits.length() < 16) {
        return {{}, {}, {}};
    }
    
    int mNum = binary_to_int(bits, 0, 8);
    int eNum = binary_to_int(bits, 8, 4);
    int sNum = binary_to_int(bits, 12, 4);
    
    int p = 16;
    std::vector<std::vector<int>> res;
    
    // Process main, extra, and side decks
    int counts[] = {mNum, eNum, sNum};
    for (int c = 0; c < 3; c++) {
        int cnt = counts[c];
        std::vector<int> lst;
        for (int i = 0; i < cnt; i++) {
            if (p + 29 > bits.length()) {
                break; // Not enough bits left
            }
            
            std::string prefix = bits.substr(p, 2);
            int cid = binary_to_int(bits, p + 2, 27);
            p += 29;
            
            int n = 1;
            if (prefix == "01") n = 1;
            else if (prefix == "10") n = 2;
            else if (prefix == "11") n = 3;
            
            for (int j = 0; j < n; j++) {
                lst.push_back(cid);
            }
        }
        res.push_back(lst);
    }
    
    return res;
}

// Main encoding function
std::string DeckCode_Encode(const std::vector<int>& main, const std::vector<int>& ex,
                           const std::vector<int>& side, const std::wstring& name) {
    auto bits_pair = encode_bits(main, ex, side);
    auto& bits = bits_pair.first;
    auto& counts = bits_pair.second;
    auto bytes = to_bytes(bits);
    return b64url_encode(bytes);
}

// Check if character is alphanumeric or underscore
bool is_alnum_or_underscore(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-';
}

// Extract value from query parameter
std::string get_query_param(const std::string& query, const std::string& param) {
    size_t pos = query.find(param + "=");
    if (pos == std::string::npos) return "";
    
    pos += param.length() + 1;
    size_t end_pos = query.find("&", pos);
    if (end_pos == std::string::npos) {
        return query.substr(pos);
    } else {
        return query.substr(pos, end_pos - pos);
    }
}

// Parse ydk content
std::vector<std::vector<int>> parse_ydk_content(const std::string& content) {
    std::vector<std::vector<int>> result{{}, {}, {}}; // main, extra, side
    
    std::istringstream iss(content);
    std::string line;
    int current_section = -1; // -1 = none, 0 = main, 1 = extra, 2 = side
    
    while (std::getline(iss, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Identify section headers
        if (line == "!main") {
            current_section = 0;
            continue;
        } else if (line == "!extra") {
            current_section = 1;
            continue;
        } else if (line == "!side") {
            current_section = 2;
            continue;
        }
        
        // Parse card ID if we're in a valid section
        if (current_section >= 0 && !line.empty()) {
            try {
                int card_id = std::stoi(line);
                if (card_id > 0) {
                    result[current_section].push_back(card_id);
                }
            } catch (...) {
                // Ignore invalid card IDs
            }
        }
    }
    
    return result;
}

// Main decoding function
bool DeckCode_Decode(const std::string& text, std::vector<int>& main,
                    std::vector<int>& ex, std::vector<int>& side) {
    // Check if this looks like a ydk file content (contains section markers)
    if (text.find("!main") != std::string::npos || 
        text.find("!extra") != std::string::npos || 
        text.find("!side") != std::string::npos) {
        auto result = parse_ydk_content(text);
        main = result[0];
        ex = result[1];
        side = result[2];
        return true;
    }
    
    // Check if it's a v0 URI (contains main_all, extra_all, or side_all)
    if (text.find("main_all=") != std::string::npos ||
        text.find("extra_all=") != std::string::npos ||
        text.find("side_all=") != std::string::npos) {
        // This is v0 format - parsing would require more complex logic
        // For now, we'll just return false as we focus on v1
        return false;
    }
    
    // Check if it's a v1 URI (contains ygo_type=deck)
    if ((text.find("ygo_type=deck") != std::string::npos || text.find("ygotype=deck") != std::string::npos)) {
        // Extract the part after '?'
        size_t query_start = text.find('?');
        if (query_start == std::string::npos) return false;
        
        std::string query = text.substr(query_start + 1);
        
        // Get the deck parameter
        std::string deck_code;
    // Prefer the real param name "d" (deck.ourygo.top), fall back to legacy "deck".
    // Require a boundary before the name so we do not match inside another value.
    for (const char* nm : { "d=", "deck=" }) {
        size_t p = query.find(nm);
        while (p != std::string::npos && p > 0 && query[p - 1] != '&' && query[p - 1] != '?')
            p = query.find(nm, p + 1);
        if (p != std::string::npos) {
            size_t e = query.find("&", p);
            deck_code = (e == std::string::npos) ? query.substr(p + std::strlen(nm))
                                                  : query.substr(p + std::strlen(nm), e - p - std::strlen(nm));
            break;
        }
    }
        if (deck_code.empty()) return false;
        
        // Decode the deck code
        try {
            auto bytes = b64url_decode(deck_code);
            std::string bits = "";
            for (auto b : bytes) {
                std::string byte_bits = int_to_binary(b, 8);
                bits += byte_bits;
            }
            
            auto result = _parse_v1_bits(bits);
            main = result[0];
            ex = result[1];
            side = result[2];
            return true;
        } catch (...) {
            return false;
        }
    }
    
    // Assume it's a pure deck code (try to decode directly)
    try {
        // Check if it looks like a base64url string (alphanumeric, -, _, no whitespace)
        std::string clean_text = text;
        clean_text.erase(std::remove_if(clean_text.begin(), clean_text.end(), ::isspace), clean_text.end());
        
        bool valid_chars = true;
        for (char c : clean_text) {
            if (!is_alnum_or_underscore(c)) {
                valid_chars = false;
                break;
            }
        }
        
        if (!valid_chars) {
            return false;
        }
        
        auto bytes = b64url_decode(clean_text);
        std::string bits = "";
        for (auto b : bytes) {
            std::string byte_bits = int_to_binary(b, 8);
            bits += byte_bits;
        }
        
        auto result = _parse_v1_bits(bits);
        main = result[0];
        ex = result[1];
        side = result[2];
        return true;
    } catch (...) {
        return false;
    }
}
