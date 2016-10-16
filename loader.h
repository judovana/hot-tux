/* Hot-babe 
 * Copyright (C) 2002 DindinX & Cyprien
 * Copyright (C) 2002 Bruno Bellamy.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the artistic License
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License for more details.
 *
 */

typedef struct
{
  gint height, width;
  gint samples, current_sample;
  GdkPixbuf **pixbuf;
} HotBabeAnim;

int load_anim( HotBabeAnim *anim, gchar *dirname );
