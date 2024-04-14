/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * parser.cpp
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#include "../include/simpleRPC/parser.h"

namespace simpleRPC
{
#define EXPECT(X, NEW_STATE)                                                                                           \
	{                                                                                                                  \
		if(ch != X) {                                                                                                  \
			hasError = true;                                                                                           \
			goto ERROR;                                                                                                \
		}                                                                                                              \
		state = NEW_STATE;                                                                                             \
		break;                                                                                                         \
	}

#define SKIP_UNTIL(X, NEW_STATE)                                                                                       \
	{                                                                                                                  \
		if(ch != X) {                                                                                                  \
			break;                                                                                                     \
		}                                                                                                              \
		state = NEW_STATE;                                                                                             \
	}

ParserResult parse(ParserSettings& settings, const char* buffer, size_t length, char nameEndsWith)
{
	auto& state = settings.state;
	/*
 * See: https://simplerpc.readthedocs.io/en/latest/protocol.html#
00000000  ff                                                 .
    00000000  73                                                 s
    00000001  69                                                 i
    00000002  6d                                                 m
    00000003  70                                                 p
    00000004  6c                                                 l
    00000005  65                                                 e
    00000006  52                                                 R
    00000007  50                                                 P
    00000008  43                                                 C
    00000009  00                                                 .
    0000000A  03 00 00                                           ...
                       3c                                           <
                       	  49                                         I
                       	     00                                       .
                       	        3a 20                                  :
                       	        3a 20  48 20 42 3b 70 69 6e 4d   ...<I.:  H B;pinM
    0000001A  6f 64 65 3a 20 53 65 74  73 20 6d 6f 64 65 20 6f   ode: Set s mode o
    0000002A  66 20 64 69 67 69 74 61  6c 20 70 69 6e 2e 20 40   f digita l pin. @
    0000003A  70 69 6e 3a 20 50 69 6e  20 6e 75 6d 62 65 72 2c   pin: Pin  number,
    0000004A  20 40 6d 6f 64 65 3a 20  4d 6f 64 65 20 74 79 70    @mode:  Mode typ
    0000005A  65 2e 00 42 3a 20 48 3b  64 69 67 69 74 61 6c 52   e..B: H; digitalR
    0000006A  65 61 64 3a 20 52 65 61  64 20 64 69 67 69 74 61   ead: Rea d digita
    0000007A  6c 20 70 69 6e 2e                                  l pin.
    00000080  20 40 70 69 6e 3a 20 50  69                         @pin: P i
    00000089  6e 20 6e 75 6d 62 65                               n numbe
    00000090  72 2e 20 40 72 65 74 75  72 6e 3a 20 50 69 6e 20   r. @retu rn: Pin
    000000A0  76 61 6c 75 65 2e 00                               value..
    000000A7  3a 20 48 20 42 3b 64 69  67 69 74 61 6c 57 72 69   : H B;di gitalWri
    000000B7  74 65 3a 20 57 72 69 74  65 20                     te: Writ e
    000000C1  74 6f 20 61 20 64 69 67  69 74 61 6c 20 70 69 6e   to a dig ital pin
    000000D1  2e 20 40 70 69 6e 3a 20  50 69 6e 20 6e 75 6d 62   . @pin:  Pin numb
    000000E1  65 72 2e 20 40 76 61 6c  75 65 3a 20 50 69 6e 20   er. @val ue: Pin
    000000F1  76 61 6c 75 65 2e 00                               value..
                                   00                                   .
*/
	bool hasError = false;
	for(const char* p = buffer; p != buffer + length; p++) {
		char ch = *p;

	REENTER:
		switch(state) {
		case ParserState::ready: {
			state = ParserState::header_s;
			/* fall through */
		}
		case ParserState::header_s:
			EXPECT('s', ParserState::header_si);
		case ParserState::header_si:
			EXPECT('i', ParserState::header_sim);
		case ParserState::header_sim:
			EXPECT('m', ParserState::header_simp);
		case ParserState::header_simp:
			EXPECT('p', ParserState::header_simpl);
		case ParserState::header_simpl:
			EXPECT('l', ParserState::header_simple);
		case ParserState::header_simple:
			EXPECT('e', ParserState::header_simpleR);
		case ParserState::header_simpleR:
			EXPECT('R', ParserState::header_simpleRP);
		case ParserState::header_simpleRP:
			EXPECT('P', ParserState::header_simpleRPC);
		case ParserState::header_simpleRPC:
			EXPECT('C', ParserState::header_end);
		case ParserState::header_end:
			EXPECT('\0', ParserState::version_major);
		case ParserState::version_major:
			// ignore for now and advance
			state = ParserState::version_minor;
			break;
		case ParserState::version_minor:
			// ignore for now and advance
			state = ParserState::version_patch;
			break;
		case ParserState::version_patch:
			// ignore for now and advance
			state = ParserState::ness;
			break;
		case ParserState::ness:
			// ignore for now and advance
			state = ParserState::type;
			break;
		case ParserState::type:
			// ignore for now and advance
			state = ParserState::type_end;
			break;
		case ParserState::type_end: {
			if(ch != '\0') {
				goto ERROR;
			}

			if(settings.startMethods) {
				settings.startMethods();
			}
			state = ParserState::extract_method_start;
			break;
		}
		case ParserState::extract_method_start: {
			if(ch == '\0') {
				state = ParserState::end_methods;
				goto REENTER;
			}

			if(settings.startMethod) {
				settings.startMethod();
			}
			state = ParserState::extract_method_signature;
			/* fall-through */
		}
		case ParserState::extract_method_signature: {
			if(ch == ';') {
				state = ParserState::extract_method_name;
				break;
			}

			if(settings.methodSignature) {
				settings.methodSignature(ch);
			}
			break;
		}
		case ParserState::extract_method_name: {
			if(ch == nameEndsWith) {
				state = ParserState::extract_method_end;
				break;
			}

			if(settings.methodName) {
				settings.methodName(ch);
			}
			break;
		}
		case ParserState::extract_method_end: {
			SKIP_UNTIL('\0', ParserState::extract_method_start);

			if(settings.endMethod) {
				settings.endMethod();
			}
			break;
		}
		case ParserState::end_methods: {
			if(settings.endMethods) {
				settings.endMethods();
			}
			state = ParserState::finished;
			/* fall through */
		}
		case ParserState::finished: {
			state = ParserState::ready;
			return ParserResult::finished;
		}
		default:
			break;
		} // end switch

	} // end for

ERROR:
	if(hasError) {
		return ParserResult::error;
	}

	return ParserResult::more;
}

String toString(ParserResult result)
{
	using namespace simpleRPC;
	switch(result) {
	case ParserResult::finished:
		return F("finished");
	case ParserResult::more:
		return F("more");
	case ParserResult::error:
		return F("error");
	}
	return nullptr;
}

} // namespace simpleRPC
