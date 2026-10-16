/*
 * EXPM - Extended Presentation Manager library
 *
 * sample/shadebtn/shbsmp.h -- resource and control IDs for the Shaded
 * Buttons sample.
 *
 * Copyright (C) 2026 The EXPM Project Authors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#ifndef SHBSMP_H
#define SHBSMP_H

/* Dialog */
#define DLG_MAIN        100

/* Push buttons */
#define BTN_RED         110
#define BTN_GREEN       111
#define BTN_BLUE        112
#define BTN_DEF         113

/* Icon and vertical buttons */
#define ICO_ICON        120
#define ICO_MINI        121
#define ICO_DIS         122
#define BTN_V1          123
#define BTN_V2          124
#define BTN_V3          125

/* Check boxes */
#define CHK_AUTO        130
#define CHK_3STATE      131
#define CHK_OFF         132

/* Radio buttons */
#define RAD_1           140
#define RAD_2           141
#define RAD_3           142

/* Status bar */
#define ST_BAR          150

/* Dialog buttons */
#define PB_OK           1
#define PB_CANCEL       2

/* Pointer resources for the icon buttons.  The icon buttons reference
 * these by number using the class' "#up[,disabled[,active]]" text
 * convention. */
#define IDP_UP          200
#define IDP_UPD         201
#define IDP_UPUP        202
#define IDP_UPUPD       203
#define IDP_DN          204
#define IDP_DND         205
#define IDP_DNDN        206
#define IDP_DNDND       207

#endif /* SHBSMP_H */
