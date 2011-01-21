/*****************************************************************************
 * x264_config.h - Configuration for compiling with Parallel Studio within Visual Studio
 *****************************************************************************
 * Copyright (C) 2010 x264 project
 *
 * Authors: Darren R. Starr <submux@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *
 * This program is also available under a commercial proprietary license.
 * For more information, contact us at licensing@x264.com.
 *****************************************************************************/


#pragma once

#if !defined(PARALLELSTUDIO)
#define PARALLELSTUDIO
#endif

#pragma warning(disable:589)

#define HAVE_VISUALIZE 1

#define X264_BIT_DEPTH 8

#define USE_WIN32_INCLUDES 1
#define HAVE_WIN32THREAD 1

#define NEED_STRCASECMP 1

#define NEED_ROUND 1

#define NEED_ISFINITE 1

#define HAVE_STDINT_H 0

#define PTW32_STATIC_LIB 0

#define HAVE_THREAD 1

#define HAVE_GPL 1

#define X264_VERSION "(drsicc)"

#define HAVE_STRING_H 1

#define HAVE_MMX 1

#define SNPRINTF _snprintf

#define HAVE_LIBMPEGTS 1

