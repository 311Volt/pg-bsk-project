def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic', '-I./lpg-gui/include', '-Isrc',
    '-Irpclib/include', '-std=c++17', '-I/usr/include', '-Ilpg-gui/axxegro/include',
    '-Ilpg-gui/axxegro/tcbspan/include'],
  }
