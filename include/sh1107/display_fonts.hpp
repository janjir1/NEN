/*!
	@file display_fonts.hpp
	@brief font data file 10 fonts. Data Vertical addressed 1-bit color displays.
	@author Gavin Lyons.
	@details 
		-#  1. pFontDefault  6 by 8
		-#  2. pFontWide  9 by 8 (NO LOWERCASE)
		-#  3. pFontpico 3 by 6
		-#  4. pFontSinclairS 8 by 8
		-#  5. pFontMega  Mega 16 by 16
		-#  6. pFontArialBold 16 by 16
		-#  7. pFontHallfetica 16 by 16
		-#  8. pFontArialRound 16 by 24
		-#  9. pFontGroTesk 16 by 32
		-#  10. pFontSixteenSeg 32 by 48 (NUMBERS ONLY + : . -)
*/

#pragma once

#include "display_data.hpp"
#include <cstdio>
#include <array>
#include <span>

// Font data is stored in std::array in font.cpp
extern const std::span<const uint8_t> pFontDefault;    /**< pFontDefault  6 by 8*/
extern const std::span<const uint8_t> pFontWide;       /**< pFontWide  9 by 8 (NO LOWERCASE) */
extern const std::span<const uint8_t> pFontPico;       /**< pFontpico 3 by 6 */
extern const std::span<const uint8_t> pFontSinclairS;  /**< pFontSinclairS 8 by 8  */
extern const std::span<const uint8_t> pFontMega;       /**< pFontMega  Mega 16 by 16*/
extern const std::span<const uint8_t> pFontArialBold;  /**< pFontArialBold 16 by 16  */
extern const std::span<const uint8_t> pFontHallfetica; /**< pFontHallfetica 16 by 16*/
extern const std::span<const uint8_t> pFontArialRound; /**< pFontArialRound 16 by 24*/
extern const std::span<const uint8_t> pFontGroTesk;    /**< pFontGroTesk 16 by 32*/
extern const std::span<const uint8_t> pFontSixteenSeg; /**< pFontSixteenSeg 32 by 48 (NUMBERS ONLY + : . -) */

/*! @brief Font class to hold font data object  */
class displaylib_fonts 
{
	public:
		
		displaylib_fonts();
		~displaylib_fonts() = default;

		DisplayRet::Ret_Codes_e setFont(std::span<const uint8_t> font);
		void setInvertFont(bool invertStatus);
		bool getInvertFont(void);

	protected:
		std::span<const uint8_t> _FontSelect = pFontDefault; /**< span to the active font,  Fonts Stored are Const */
		uint8_t _Font_X_Size = 0x06; /**< Width Size of a Font character */
		uint8_t _Font_Y_Size = 0x08; /**< Height Size of a Font character */
		uint8_t _FontOffset = 0x00; /**< Offset in the ASCII table 0x00 to 0xFF, where font begins */
		uint8_t _FontNumChars = 0xFE; /**< Number of characters in font 0x00 to 0xFE */
	private:
		bool _FontInverted = false; /**< Is the font inverted , False = normal , true = inverted*/
};

