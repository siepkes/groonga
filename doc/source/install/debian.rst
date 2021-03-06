.. -*- rst -*-

.. highlightlang:: none

Debian GNU/Linux
================

This section describes how to install Groonga related deb packages on
Debian GNU/Linux. You can install them by ``apt``.

We distribute both 32-bit and 64-bit packages but we strongly
recommend a 64-bit package for server. You should use a 32-bit package
just only for tests or development. You will encounter an out of
memory error with a 32-bit package even if you just process medium
size data.

jessie
------

.. versionadded:: 5.0.3

Add the Groonga apt repository.

/etc/apt/sources.list.d/groonga.list::

  deb https://packages.groonga.org/debian/ jessie main
  deb-src https://packages.groonga.org/debian/ jessie main

Install::

  % sudo apt-get install apt-transport-https
  % sudo apt-get update
  % sudo apt-get install -y --allow-unauthenticated groonga-keyring
  % sudo apt-get update
  % sudo apt-get install -y -V groonga

.. include:: server-use.inc

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt-get install -y -V groonga-tokenizer-mecab

If you want to use ``TokenFilterStem`` as a token filter, install
groonga-token-filter-stem package.

Install groonga-token-filter-stem package::

  % sudo apt-get install -y -V groonga-token-filter-stem

There is a package that provides `Munin
<http://munin-monitoring.org/>`_ plugins. If you want to monitor
Groonga status by Munin, install groonga-munin-plugins package.

Install groonga-munin-plugins package::

  % sudo apt-get install -y -V groonga-munin-plugins

There is a package that provides MySQL compatible normalizer as
a Groonga plugin.
If you want to use that one, install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo apt-get install -y -V groonga-normalizer-mysql

.. note::

   If you use systemd as init, current version of Groonga does not support
   to register start-up service automatically during installation.
   If you want to register groonga-httpd/groonga-server-gqtp as
   a automatic start-up service, please execute the following commands:

   For groonga-httpd:

     % sudo systemctl enable groonga-httpd

   For groonga-server-gqtp:

     % sudo systemctl enable groonga-server-gqtp

stretch
-------

.. versionadded:: 7.0.5

Add the Groonga apt repository.

/etc/apt/sources.list.d/groonga.list::

  deb https://packages.groonga.org/debian/ stretch main
  deb-src https://packages.groonga.org/debian/ stretch main

Install::

  % sudo apt install -y -V apt-transport-https
  % sudo apt update --allow-insecure-repositories
  % sudo apt install -y -V --allow-unauthenticated groonga-keyring
  % sudo apt update
  % sudo apt install -y -V groonga

.. include:: server-use.inc

If you want to use `MeCab <https://taku910.github.io/mecab/>`_ as a
tokenizer, install groonga-tokenizer-mecab package.

Install groonga-tokenizer-mecab package::

  % sudo apt install -y -V groonga-tokenizer-mecab

If you want to use ``TokenFilterStem`` as a token filter, install
groonga-token-filter-stem package.

Install groonga-token-filter-stem package::

  % sudo apt install -y -V groonga-token-filter-stem

There is a package that provides `Munin
<http://munin-monitoring.org/>`_ plugins. If you want to monitor
Groonga status by Munin, install groonga-munin-plugins package.

Install groonga-munin-plugins package::

  % sudo apt install -y -V groonga-munin-plugins

There is a package that provides MySQL compatible normalizer as
a Groonga plugin.
If you want to use that one, install groonga-normalizer-mysql package.

Install groonga-normalizer-mysql package::

  % sudo apt install -y -V groonga-normalizer-mysql

.. note::

   If you use systemd as init, current version of Groonga does not support
   to register start-up service automatically during installation.
   If you want to register groonga-httpd/groonga-server-gqtp as
   a automatic start-up service, please execute the following commands:

   For groonga-httpd:

     % sudo systemctl enable groonga-httpd

   For groonga-server-gqtp:

     % sudo systemctl enable groonga-server-gqtp

Build from source
-----------------

Install required packages to build Groonga for Debian jessie::

  % sudo apt-get install -y -V  wget tar build-essential pkg-config zlib1g-dev libmsgpack-dev libzmq3-dev libevent-dev libmecab-dev liblz4-dev

Install required packages to build Groonga for Debian stretch::

  % sudo apt-get install -y -V  wget tar build-essential pkg-config zlib1g-dev libmsgpack-dev libzmq3-dev libevent-dev libmecab-dev liblz4-dev libzstd-dev

Download source::

  % wget https://packages.groonga.org/source/groonga/groonga-8.0.4.tar.gz
  % tar xvzf groonga-8.0.4.tar.gz
  % cd groonga-8.0.4

Configure (see :ref:`source-configure` about ``configure`` options)::

  % ./configure

Build::

  % make -j$(grep '^processor' /proc/cpuinfo | wc -l)

Install::

  % sudo make install
