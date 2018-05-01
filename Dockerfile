FROM fedora:28

RUN dnf update -y && dnf install -y lua && dnf install -y libuv
RUN dnf install -y lua-devel && dnf install -y mongo-c-driver && dnf install -y libstdc++

RUN useradd -d /home/luser -g wheel -ms /bin/bash luser

ADD ./luatest /home/luser/luatest
ADD ./skeleton.so /home/luser/skeleton.so
ADD ./mongo.so /home/luser/mongo.so
ADD ./config.lua /home/luser/config.lua

RUN ln -s /usr/lib64/liblua-5.3.so /usr/lib64/liblua.so.5.3
RUN chown -R luser /home/luser

USER luser
WORKDIR /home/luser
CMD /home/luser/luatest
