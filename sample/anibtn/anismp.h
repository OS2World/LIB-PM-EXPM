/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/anibtn/anismp.h -- dialog, control and bitmap IDs for the
 * AniButtn sample application (anismp.c / anismp.rc).
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#ifndef ANISMP_H
#define ANISMP_H

/* Dialog template */
#define DLG_TOOLBAR     100

/* Animated push buttons */
#define TBB_FILE        101
#define TBB_NEW         102
#define TBB_EXIT        103
#define TBB_NEXT        104
#define TBB_LAST        105
#define TBB_NA1         106
#define TBB_NA2         107
#define TBB_NA3         108
#define TBB_NA4         109
#define TBB_NA5         110
#define TBB_NA6         111
#define TBB_NA7         112

/* Status-text field */
#define TX_HELP         120

/* Bitmap resources (up / down pairs) */
#define BMP_ABT_UP      201
#define BMP_ABT_DN      202
#define BMP_NXT_UP      203
#define BMP_NXT_DN      204
#define BMP_LST_UP      205
#define BMP_LST_DN      206
#define BMP_INF_UP      207
#define BMP_INF_DN      208
#define BMP_EXT_UP      209
#define BMP_EXT_DN      210
#define BMP_NA_UP       211
#define BMP_NA_DN       212

/* Bitmap size in pels, passed to WinCreateAniButton */
#define BMP_CX          68
#define BMP_CY          31

#endif /* ANISMP_H */
