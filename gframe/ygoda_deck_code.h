#pragma once
#include <string>
#include <vector>

// Generate deck code (return pure code; name can be empty)
std::string DeckCode_Encode(const std::vector<int>& main, const std::vector<int>& ex,
                            const std::vector<int>& side, const std::wstring& name = L"");
// Decode deck code (supports: pure code / full URI(ygo_type=deck) / v0(main_all…) / ydk text)
bool DeckCode_Decode(const std::string& text, std::vector<int>& main,
                     std::vector<int>& ex, std::vector<int>& side);