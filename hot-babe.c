/* Hot-babe 
 * Copyright (C) 2002 DindinX <David@dindinx.org>
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
 * this code is using some ideas from wmbubble (timecop@japan.co.jp)
 *
 */

/* general includes */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef __FreeBSD__
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#ifndef CPUSTATES                                                              
#include <sys/dkstat.h>
#endif
#endif                                                                         

/* x11 includes */
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "loader.h"

static int system_cpu(void);
static void hotbabe_setup_samples(void);
static void hotbabe_update(void);
static void create_hotbabe_window(void);
static void print_usage(void);

/* global variables */

typedef struct
{
  /* X11 stuff */
  GdkWindow *win;        /* main window */
  HotBabeAnim anim;
  gint x,y;
  guchar **pixels;
  guchar *dest;

  int samples;

  /* CPU percentage stuff.  soon to go away */
  int loadIndex;
  u_int64_t *load, *total;
  guint threshold;

  /* optional stuff */
  gboolean incremental;
  gboolean noNice;
  guint    delay;  
} HotBabeData;

HotBabeData bm;

#if 0
/* FIXME New BSD and Solaris code.. to check.
 * doesn't work with Linux (getloadavg return 1.000) */
static int system_cpu(void)
{
  int rc;
  double loadavg[15];
  rc=getloadavg(loadavg, 1); 
  while( rc-- )
    printf( "load = %f\n", loadavg[rc] );
  rc=100*loadavg[0];
  return rc;
}
#endif

/* returns current CPU load in percent, 0 to 256 */
static int system_cpu(void)
{
  unsigned int  cpuload;
  int           i;
#ifdef __linux__
  u_int64_t     load, total, oload, ototal;
  u_int64_t     ab, ac, ad, ae;
  FILE         *stat;
#endif
#ifdef __FreeBSD__
  long load, total, oload, ototal;
  long ab, ac, ad, ae;
  long cp_time[CPUSTATES];
  size_t len = sizeof(cp_time);
#endif

#ifdef __linux__
  stat = fopen("/proc/stat", "r");
  fscanf(stat, "%*s %Ld %Ld %Ld %Ld", &ab, &ac, &ad, &ae);
  fclose(stat);
#endif
#ifdef __FreeBSD__
  if (sysctlbyname("kern.cp_time", &cp_time, &len, NULL, 0) < 0)
    (void)fprintf(stderr, "Cannot get kern.cp_time");

  ab = cp_time[CP_USER];
  ac = cp_time[CP_NICE];
  ad = cp_time[CP_SYS];
  ae = cp_time[CP_IDLE];
#endif


  /* Find out the CPU load */
  /* user + sys = load
   * total = total */
  load = ab + ad;  /* cpu.user + cpu.sys; */
  if(!bm.noNice) load += ac;
  total = ab + ac + ad + ae;  /* cpu.total; */

  i = bm.loadIndex;
  oload = bm.load[i];
  ototal = bm.total[i];

  bm.load[i] = load;
  bm.total[i] = total;
  bm.loadIndex = (i + 1) % bm.samples;

  /*
   *   Because the load returned from libgtop is a value accumulated
   *   over time, and not the current load, the current load percentage
   *   is calculated as the extra amount of work that has been performed
   *   since the last sample. yah, right, what the fuck does that mean?
   */
  if (ototal == 0 || total==ototal)    /* ototal == 0 means that this is the first time we get here */
    cpuload = 0;
  else
    cpuload = (256 * (load - oload)) / (total - ototal);

  return cpuload;
}

GdkPixmap     *pixmap;
GdkGC *gc;

/* This is the function that actually creates the display widgets */
static void create_hotbabe_window(void)
{
#define MASK GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
  GdkWindowAttr  attr;
  GdkBitmap     *mask;
  GdkScreen *defscrn;

  bm.anim.width = gdk_pixbuf_get_width(bm.anim.pixbuf[0]);
  bm.anim.height = gdk_pixbuf_get_height(bm.anim.pixbuf[0]);
  defscrn=gdk_screen_get_default();

  attr.width = bm.anim.width;
  attr.height = bm.anim.height;
  if( bm.x < 0 ) bm.x += 1 + gdk_screen_get_width(defscrn) - attr.width;
  if( bm.y < 0 ) bm.y += 1 + gdk_screen_get_height(defscrn) - attr.height;
  attr.x = bm.x;
  attr.y = bm.y;
  attr.title = "hot-babe";
  attr.event_mask = MASK;
  attr.wclass = GDK_INPUT_OUTPUT;
  attr.visual = gdk_visual_get_system();
  attr.colormap = gdk_colormap_get_system();
  attr.wmclass_name = "hot-babe";
  attr.wmclass_class = "hot-babe";
  attr.window_type = GDK_WINDOW_TOPLEVEL;

  bm.win = gdk_window_new(NULL, &attr,
      GDK_WA_TITLE | GDK_WA_WMCLASS |
      GDK_WA_VISUAL | GDK_WA_COLORMAP |
      GDK_WA_X | GDK_WA_Y);
  if (!bm.win)
  {
    fprintf(stderr, "Cannot make toplevel window\n");
    exit (-1);
  }
  gdk_window_set_decorations(bm.win, 0);
  gdk_window_set_skip_taskbar_hint(bm.win, TRUE);
  gdk_window_set_skip_pager_hint(bm.win, TRUE); 
  gdk_window_set_type_hint(bm.win, GDK_WINDOW_TYPE_HINT_DOCK);
//  gdk_window_set_keep_below(bm.win, TRUE);

  gdk_pixbuf_render_pixmap_and_mask( bm.anim.pixbuf[bm.anim.samples-1], &pixmap, &mask, 127 );

  gdk_window_shape_combine_mask(bm.win, mask, 0, 0);
  gdk_pixbuf_render_pixmap_and_mask( bm.anim.pixbuf[0], &pixmap, &mask, 127 );
  gdk_window_set_back_pixmap(bm.win, pixmap, False);

  gdk_window_show(bm.win);

  gc = gdk_gc_new (pixmap);
#undef MASK
}

static void hotbabe_update(void)
{
  guint   loadPercentage;
  static guint   old_percentage = 0;
  guint   i;
  guchar *pixels1, *pixels2, *src1, *src2, *dest;
  static gint    robinet = 0;

  /* Find out the CPU load */
  loadPercentage = system_cpu();

  if (bm.threshold)
  {
    if (loadPercentage < bm.threshold || bm.threshold>255)
      loadPercentage = 0;
    else
      loadPercentage = (loadPercentage-bm.threshold)*256/(256-bm.threshold);
  }

  robinet +=loadPercentage/50-3;

  robinet = CLAMP(robinet, 0, 256);

  if (bm.incremental)
    loadPercentage = robinet;

  if (loadPercentage != old_percentage)
  {
    gint range = 256  / (bm.anim.samples-1);
    gint index = loadPercentage/range;

    old_percentage = loadPercentage;
    if  (index>bm.anim.samples-1) index = bm.anim.samples-1;
    pixels1 = bm.pixels[index];
    if (index  == bm.anim.samples-1) pixels2 = bm.pixels[index];
    else pixels2 = bm.pixels[index+1];

    loadPercentage = loadPercentage % range;
    dest = bm.dest; src1 = pixels1; src2 = pixels2;
    for (i=0  ;  i<bm.anim.height*bm.anim.width ;  i++)
    {
      guint val, j;

      if (src1[3])
      {
        for (j=0;j<3;j++)
        {
          val = ((guint)*(src2++))*loadPercentage+((guint)*(src1++))*(range-loadPercentage);
          *(dest++) = val/range;
          //*(dest++) = (val >> 6);  // bad hack!
        }
        src1++;
        src2++;
      } else
      {
        src1+=4;src2+=4;dest+=3;
      }
    }

    gdk_draw_rgb_image(pixmap, gc, 0, 0, bm.anim.width, bm.anim.height, GDK_RGB_DITHER_NONE,
        bm.dest, 3 * bm.anim.width);
    gdk_window_set_back_pixmap(bm.win, pixmap, False);
    gdk_window_clear(bm.win);
  }
}

static void hotbabe_setup_samples(void)
{
  int       i;
  u_int64_t load = 0, total = 0;

  bm.loadIndex = 0;
  bm.load = malloc(bm.samples * sizeof(u_int64_t));
  bm.total = malloc(bm.samples * sizeof(u_int64_t));
  for (i = 0; i < bm.samples;i++)
  {
    bm.load[i] = load;
    bm.total[i] = total;
  }
}

static void print_version(void)
{
  g_print("hot-babe version " VERSION "\n\n");
}

static void print_usage(void)
{
  g_print("Usage: hot-babe [OPTIONS]\n\n");
  g_print("OPTIONS are from the following:\n\n");
  g_print(" -t, --threshold n    use only the first picture before n%%.\n");
  g_print(" -i, --incremental    incremental (slow) mode.\n");
  g_print(" -d, --delay  n       update every n millisecondes.\n");
  g_print(" -h, --help           show this message and exit.\n");
  g_print(" -N, --noNice         don't count nice time in usage.\n");
  g_print(" -n, --nice  n        set self-nice to n.\n");
  g_print("     --dir directory  use images from directory.\n");
  g_print("     --geometry {+|-}x{+|-}y  position the hot-babe.\n");
  g_print(" -v, --version        show version and exit.\n");
}

void parse_geometry( char *arg )
{
  char sign[2];
  guint val[2];
  int i = 0;
  
  i = sscanf( arg, "%c%u%c%u", &sign[0], &val[0], &sign[1], &val[1] );
  if( i != 4 )
    return;
  
  if( sign[0] == '+' ) bm.x = val[0];
  if( sign[0] == '-' ) bm.x = -1-val[0];
  if( sign[1] == '+' ) bm.y = val[1];
  if( sign[1] == '-' ) bm.y = -1-val[1];
}

int main(int argc, char **argv)
{
  GdkEvent *event;

  gint      i;
  gchar *dir;
  char conf[256];
  FILE *f;

  /* initialize GDK */
  if (!gdk_init_check(&argc, &argv))
  {
    fprintf(stderr,
        "GDK init failed, bye bye.  Check \"DISPLAY\" variable.\n");
    exit(-1);
  }
  gdk_rgb_init();

  /* zero data structure */
  memset(&bm, 0, sizeof(bm));

  bm.samples     = 16;
  bm.incremental = FALSE;
  bm.delay       = 15000;
  bm.noNice      = FALSE;
  bm.x = -1;
  bm.y = -1;

  dir            = NULL;


  snprintf( conf, 256, "%s/.hot-babe/config", g_get_home_dir() );
  f = fopen( conf, "r" );
  if( f )
  {
    char line[256], *l;
    guint uval;
    gint val;
    char sval[260];

    while( (l=fgets( line, 255, f )) )
    {
      while( *l )
      {
        if( *l == '\n' || *l == '#' ) *l = 0;
        l++;
      }
      if( !*line ) continue;

      if( sscanf( line, "threshold %u", &uval ) == 1 )
      {
        bm.threshold = uval*256/100;
        bm.threshold = MIN (255, bm.threshold);
      } else if ( !strcmp(line, "incremental") )
      {
        bm.incremental = TRUE;
      } else if (!strcmp(line, "noNice") )
      {
        bm.noNice = TRUE;
      } else if ( sscanf( line, "nice %d", &val) == 1 )
      {
        nice( val );
      } else if ( sscanf( line, "delay %u", &uval) == 1 )
      {
        bm.delay = uval*1000;
      } else if ( sscanf( line, "dir %s", sval) == 1)
      {
        dir = strdup( sval );
      } else if ( sscanf( line, "geometry %s", sval) == 1)
      {
        parse_geometry( sval );
      }
    }
    fclose(f);
  }

  for (i=1 ; i<argc ; i++)
  {
    if (!strcmp(argv[i], "--threshold") || !strcmp(argv[i], "-t"))
    {
      i++;
      if  (i<argc)
      {
        bm.threshold = atoi(argv[i])*256/100;
        bm.threshold = MIN (255, bm.threshold);
      }        
    } else if (!strcmp(argv[i], "--version") || !strcmp(argv[i], "-v"))
    {
      print_version();
      exit(0);
    } else if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h"))
    {
      print_usage();
      exit(0);
    } else if (!strcmp(argv[i], "--incremental") || !strcmp(argv[i], "-i"))
    {
      bm.incremental = TRUE;
    } else if (!strcmp(argv[i], "--noNice") || !strcmp(argv[i], "-N"))
    {
      bm.noNice = TRUE;
    } else if (!strcmp(argv[i], "--nice") || !strcmp(argv[i], "-n"))
    {
      i++;
      if  (i<argc)
      {
        nice( atoi(argv[i]) );
      }
    } else if (!strcmp(argv[i], "--delay") || !strcmp(argv[i], "-d"))
    {
      i++;
      if  (i<argc)
      {
        bm.delay = atoi(argv[i])*1000;
      }        
    } else if (!strcmp(argv[i], "--dir"))
    {
      i++;
      if  (i<argc)
      {
        dir = argv[i];
      }        
    } else if (!strcmp(argv[i], "--geometry"))
    {
      i++;
      if  (i<argc)
      {
        parse_geometry( argv[i] );
      }        
    }
  }

  if( dir != NULL ) {
    char path[256], home[256];
    snprintf( path, 256, PREFIX "/share/hot-babe/%s", dir );
    snprintf( home, 256, "%s/.hot-babe/%s", g_get_home_dir(), dir );
    if( load_anim( &bm.anim, path ) &&
        load_anim( &bm.anim, home ) &&
        load_anim( &bm.anim, dir ) ) {
      fprintf( stderr, "Can't find pictures\n" );
      return 1;
    }
  } else {
    if( load_anim( &bm.anim, PREFIX "/share/hot-babe/hb01" ) &&
        load_anim( &bm.anim, "hb01" ) ) {
      fprintf( stderr, "Can't find pictures\n" );
      return 1;
    }
  }
  create_hotbabe_window();

  bm.pixels = malloc( sizeof(guchar*) * bm.anim.samples );
  for( i = 0 ; i < bm.anim.samples ; i++ )
    bm.pixels[i] = gdk_pixbuf_get_pixels( bm.anim.pixbuf[i] );
  bm.dest = malloc( bm.anim.width * bm.anim.height * 3 );

  hotbabe_setup_samples();

  while (1)
  {
    while (gdk_events_pending())
    {
      event = gdk_event_get();
      if (event)
      {
        switch (event->type)
        {
          case GDK_DESTROY:
            gdk_exit(0);
            exit(0);
            break;
          case GDK_BUTTON_PRESS:
            if (event->button.button == 3)
            {
              exit(0);
              break;
            }
            break;
          default:
            break;
        }
      }
    }
    usleep(bm.delay);
    hotbabe_update();
  }
  return 0;
}

