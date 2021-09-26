#include "g3log/ColorCoutSink.hpp"
#include <set>


namespace g3 {

   // ======================== Foreground Colors ==============================
   const TextAttribute FG_black = termcolor::black;
   const TextAttribute FG_gray = termcolor::grey; // bright black
   const TextAttribute FG_grey = termcolor::grey; // bright black

   const TextAttribute FG_red = termcolor::red;
   const TextAttribute FG_green = termcolor::green;
   const TextAttribute FG_yellow = termcolor::yellow;
   const TextAttribute FG_blue = termcolor::blue;
   const TextAttribute FG_magenta = termcolor::magenta;
   const TextAttribute FG_cyan = termcolor::cyan;
   const TextAttribute FG_white = termcolor::white;
   // bright counterparts
   const TextAttribute FG_redB = termcolor::bright_red;
   const TextAttribute FG_greenB = termcolor::bright_green;
   const TextAttribute FG_yellowB = termcolor::bright_yellow;
   const TextAttribute FG_blueB = termcolor::bright_blue;
   const TextAttribute FG_magentaB = termcolor::bright_magenta;
   const TextAttribute FG_cyanB = termcolor::bright_cyan;
   const TextAttribute FG_whiteB = termcolor::bright_white;
   // ======================== Background Colors ==============================
   const TextAttribute BG_black = termcolor::on_black;
   const TextAttribute BG_gray = termcolor::on_grey; // bright black
   const TextAttribute BG_grey = termcolor::on_grey; // bright black
   
   const TextAttribute BG_red = termcolor::on_red;
   const TextAttribute BG_green = termcolor::on_green;
   const TextAttribute BG_yellow = termcolor::on_yellow;
   const TextAttribute BG_blue = termcolor::on_blue;
   const TextAttribute BG_magenta = termcolor::on_magenta;
   const TextAttribute BG_cyan = termcolor::on_cyan;
   const TextAttribute BG_white = termcolor::on_white;
   // bright counterparts
   const TextAttribute BG_redB = termcolor::on_bright_red;
   const TextAttribute BG_greenB = termcolor::on_bright_green;
   const TextAttribute BG_yellowB = termcolor::on_bright_yellow;
   const TextAttribute BG_blueB = termcolor::on_bright_blue;
   const TextAttribute BG_magentaB = termcolor::on_bright_magenta;
   const TextAttribute BG_cyanB = termcolor::on_bright_cyan;
   const TextAttribute BG_whiteB = termcolor::on_bright_white;
   // ======================== Attribute Manipulators =========================
   // Windows API only supports underline and reverse
   const TextAttribute ATTR_bold = termcolor::bold;
   const TextAttribute ATTR_dark = termcolor::dark;
   const TextAttribute ATTR_italic = termcolor::italic;
   const TextAttribute ATTR_underline = termcolor::underline;
   const TextAttribute ATTR_blink = termcolor::blink;
   const TextAttribute ATTR_reverse = termcolor::reverse;
   const TextAttribute ATTR_hide = termcolor::concealed;
   const TextAttribute ATTR_crossed = termcolor::crossed;
   // ============================ Reset Manipulator ===========================
   const TextAttribute reset = termcolor::reset;

   
   const std::set<TextAttribute> FG_COLORS = { FG_black, FG_gray,
	  FG_red, FG_green, FG_yellow, FG_blue, FG_magenta, FG_cyan, FG_white, 
	  FG_redB, FG_greenB, FG_yellowB, FG_blueB, FG_magentaB, FG_cyanB, FG_whiteB, 
   };

   const std::set<TextAttribute> BG_COLORS = { BG_black, BG_gray,
	  BG_red, BG_green, BG_yellow, BG_blue, BG_magenta, BG_cyan, BG_white,
	  BG_redB, BG_greenB, BG_yellowB, BG_blueB, BG_magentaB, BG_cyanB, BG_whiteB,
   };

   const std::set<TextAttribute> ATTRIBUTES = { ATTR_bold, ATTR_dark, ATTR_italic, 
	  ATTR_underline, ATTR_blink, ATTR_reverse, ATTR_hide, ATTR_crossed
   };

} // namespace g3
