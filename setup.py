import subprocess

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class script_build_ext(build_ext):
    def build_extensions(self):
        subprocess.check_call(['./build.sh'])
        super().build_extensions()


setup(
    name="pyj",
    version="0.1",
    ext_modules=[Extension("pyj", ['pyj.c'], extra_link_args=['libj.a'])],
    cmdclass={'build_ext': script_build_ext},
)
