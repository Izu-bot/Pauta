pauta_sources = [
  'main.c',
  'pauta-application.c',
  'pauta-window.c',
]

pauta_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
]

pauta_sources += gnome.compile_resources('pauta-resources',
  'pauta.gresource.xml',
  c_name: 'pauta'
)

executable('pauta', pauta_sources,
  dependencies: pauta_deps,
       install: true,
)
