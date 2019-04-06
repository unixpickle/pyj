# pyj

This is a small Python API to execute J source code.

# Usage

Install the module like so:

```
git clone --recurse-submodules https://github.com/unixpickle/pyj.git
cd pyj
pip install -e .
```

This will install a `pyj` module with a `Runtime` object. The `Runtime` object has two methods: `do` and `set_output`. The former runs code, while the latter sets the callback for evaluation results.

Here's a simple example:

```
>>> from pyj import Runtime
>>> rt = Runtime()
>>> rt.set_output(lambda code, text: print(text.rstrip()))
>>> rt.do('3+5')
8
0
>>> rt.do('aoesuth')
|value error: aoesuth
21
```
