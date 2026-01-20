cmake searchengine -B searchengine/build
cmake --build searchengine/build
cmake --install searchengine/build --prefix $(python -c "import site; print(site.getsitepackages()[0])") 