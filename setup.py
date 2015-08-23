from distutils.core import setup, Extension

module1 = Extension('demo',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['./src/include/'],
                    libraries = ['tcl83'],
                    library_dirs = ['./src/lib/'],
                    sources = ['./src/tis.c'])

setup (name = 'PackageName',
       version = '1.0',
       description = 'This is a demo package',
       author = '',
       author_email = '',
       url = '',
       long_description = ''' ''',ext_modules = [])