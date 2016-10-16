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

/* general includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* filesystem includes */
#include <sys/types.h>
#include <sys/stat.h>

/* x11 includes */
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "loader.h"

/* Animation loader from a directory */
int load_anim( HotBabeAnim *anim, gchar *dirname )
{
  struct stat buf;
  int ret;
  FILE *fd;
  char filename[1024];
  char line[1024];
  int i;

  anim->samples = 0;
  anim->current_sample = 0;

  /* dirname must be a valid directory */

  if( dirname == NULL ) return 1;
  if( (ret = stat( dirname, &buf )) < 0 ) {
    return 1;
  }
  if( !S_ISDIR(buf.st_mode) ) {
    return 2;
  }

  /* description file */
  
  strncpy( filename, dirname, 512 );
  strcat( filename, "/descr" );
  if( (fd = fopen( filename, "r" )) == NULL ) {
    perror( filename );
    return 3;
  }

  if( fgets( line, 1023, fd ) == NULL )
  {
    fclose( fd );
    perror( "--" );
    return 4;
  }

  if( (anim->samples = atoi( line )) == 0 ) {
    fclose( fd );
    fprintf( stderr, "%s: bad file format\n", filename );
    return 4;
  }

  /* load images */
  
  anim->pixbuf = malloc( sizeof(GdkPixbuf*) * anim->samples );
  for( i = 0 ; i < anim->samples ; i++ ) {
    if( fgets( line, 512, fd ) == NULL ) {
      fclose( fd );
      perror( "--" );
      return 5;
    }
    if( line[strlen(line)-1] == '\n' )
      line[strlen(line)-1] = '\0';
    strncpy( filename, dirname, 512 );
    strcat( filename, "/" );
    strcat( filename, line );
    /* FIXME: I'm a bad boy, I should use a real GError -temsa */
    anim->pixbuf[i] = gdk_pixbuf_new_from_file( filename, NULL );
  }

  fclose( fd );

  anim->width  = gdk_pixbuf_get_width( anim->pixbuf[0]);
  anim->height = gdk_pixbuf_get_height(anim->pixbuf[0]);

  return 0;
}
