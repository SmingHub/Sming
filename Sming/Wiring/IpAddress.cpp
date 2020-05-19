/*
  IpAddress.cpp - Base class that provides IpAddress
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "IpAddress.h"
#include "Print.h"

void IpAddress::fromString(const String& address)
{
    ipaddr_aton(address.c_str(), &(this->address));
}

size_t IpAddress::printTo(Print& p) const
{
    return p.print(toString());
}

String IpAddress::toString() const
{
    char text[16];
    ipaddr_ntoa_r(&address, text, 16);
    return text;
}
