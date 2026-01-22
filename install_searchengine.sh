cmake searchengine -B searchengine/build
cmake --build searchengine/build --config Release
cmake --install searchengine/build --prefix $(python -c "import site; print(site.getsitepackages()[0])") 