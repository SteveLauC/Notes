Postgres has a binary `pg_config` for reporting all the installation paths:

```c
static const InfoItem info_items[] = {
	{"--bindir", "BINDIR"},
	{"--docdir", "DOCDIR"},
	{"--htmldir", "HTMLDIR"},
	{"--includedir", "INCLUDEDIR"},
	{"--pkgincludedir", "PKGINCLUDEDIR"},
	{"--includedir-server", "INCLUDEDIR-SERVER"},
	{"--libdir", "LIBDIR"},
	{"--pkglibdir", "PKGLIBDIR"},
	{"--localedir", "LOCALEDIR"},
	{"--mandir", "MANDIR"},
	{"--sharedir", "SHAREDIR"},
	{"--sysconfdir", "SYSCONFDIR"},
	{"--pgxs", "PGXS"},
	{"--configure", "CONFIGURE"},
	{"--cc", "CC"},
	{"--cppflags", "CPPFLAGS"},
	{"--cflags", "CFLAGS"},
	{"--cflags_sl", "CFLAGS_SL"},
	{"--ldflags", "LDFLAGS"},
	{"--ldflags_ex", "LDFLAGS_EX"},
	{"--ldflags_sl", "LDFLAGS_SL"},
	{"--libs", "LIBS"},
	{"--version", "VERSION"},
	{NULL, NULL}
};
```

How this thing works is tricky, ~~it relies on relative paths. For example, the 
`BINDIR` is guaranteed to be the parent path of `which pg_config` if everything
is correct~~:

```sh
$ which pg_config
/opt/homebrew/opt/postgresql@17/bin/pg_config

$ pg_config --bindir
/opt/homebrew/Cellar/postgresql@17/17.5/bin
```


QUES: Looks like it is more complex than I thought, figure out how this actually 
works.
