Install docker
==============

Visit the official `Docker Installation
Page <https://docs.docker.com/engine/installation/>`__ and follow the
instructions tailored for your operating system.

Install docker-compose
======================

Docker Compose makes dealing with the orchestration processes of Docker
containers (such as starting up, shutting down, and setting up
intra-container linking and volumes) really easy.

With docker compose we can define the entire multi-container application
in single file and then the application can be spinned up using one
command.

Visit the official `Docker Compose Installation
Page <https://docs.docker.com/compose/install/>`__ and follow the
instructions tailored for your operating system.

Adjust your settings
====================

::

   sming-ide:

    build: .
    
    volumes:
      - ../../Sming/:/opt/sming/
      
    ports:
    #choose a free port to connect to the web C9 editor
    - "10080:80"
    
    devices:
     # uncomment to map your serial USB adaptor 
     #- "/dev/ttyUSB0:/dev/ttyUSB0"

    
    privileged: true

Start your container
====================

.. code:: shell

   cd /opt/sming/docker/ && docker-compose -f sming-ide.yml up -d

Open your browser
=================

http://localhost:10080

|C9 screenshot example|

.. code:: shell

   cd /opt/sming/Sming
   make

.. code:: shell

   cd /opt/sming/samples/Basic_Blink
   make
   make flash

.. |C9 screenshot example| image:: images/c9-1.png
   :target: images/c9-1.png
