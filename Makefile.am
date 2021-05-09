#AM_INIT_AUTOMAKE([1.11 silent-rules subdir-objects])

#SUBDIRS = build

AM_CFLAGS = -I/opt/Wt/include -I/usr/local/include
AM_CPPFLAGS = -I/opt/Wt/include -I/usr/local/include
AM_LDFLAGS = -L/usr/local/lib -L/opt/Wt/lib
LDADD = -lwtdbo -lwtdbopostgres -lwtdbosqlite3

bin_PROGRAMS = example1
example1_SOURCES = src/example1.cc
