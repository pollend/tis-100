from distutils.core import setup, Extension

module1 = Extension('tis',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['src/headers'],
                    sources = [
                    'src/tis.c',
                    'src/source/program.c',
                    'src/source/instruction.c',
                    'src/source/node.c',
                    'src/source/util.c',
                    'src/source/input_code.c',
                    'src/source/node_list.c'])

setup (name = 'tis-100',
       version = '1.0',
       description = 'This is a demo package',
       author = '',
       author_email = '',
       url = '',
       long_description = ''' ''',ext_modules = [module1])