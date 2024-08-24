FROM nstrumenta/data-job-runner

RUN pip install conan
RUN apt install cmake -y

RUN conan profile detect
RUN sed -i 's/compiler.cppstd=gnu14/compiler.cppstd=gnu17/' /root/.conan2/profiles/default   

RUN apt install -y gdb