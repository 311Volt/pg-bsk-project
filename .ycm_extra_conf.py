def Settings( **kwargs ):
  return {
    'flags': ['-x', 'c++', '-Wall', '-pedantic', '-Isrc', '-Irpclib/include',
    '-std=c++20', '-I/usr/include'],
  }