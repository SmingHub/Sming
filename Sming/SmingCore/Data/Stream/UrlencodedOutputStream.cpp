/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UrlencodedOutputStream.cpp
 *
 * @author Slavey Karadzhov <slaff@attachix.com>
 *
 ****/

#include "UrlencodedOutputStream.h"
#include "../Services/WebHelpers/escape.h"

/*
 * @todo Revise this so stream produces encoded output line-by-line, rather than all at once.
 * Can use StreamTransformer to do this.
 */

UrlencodedOutputStream::UrlencodedOutputStream(const HttpParams& params)
{
	stream.print(params);
}
