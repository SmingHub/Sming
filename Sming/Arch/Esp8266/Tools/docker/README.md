## Install docker

Visit the official [Docker Installation Page](https://docs.docker.com/engine/installation/) and follow the instructions tailored for your operating system. 
 
## Install docker-compose

Docker Compose makes dealing with the orchestration processes of Docker containers (such as starting up, shutting down, and setting up intra-container linking and volumes) really easy. 

With docker compose we can define the entire multi-container application in single file and then the application can be spinned up using one command.

Visit the official [Docker Compose Installation Page](https://docs.docker.com/compose/install/) and follow the instructions tailored for your operating system. 

## (Optional) Adjust your setting

Edit the docker-compose.yml and adjust the settings, if needed.

```
 
# Uncomment the lines below if you want to map your local source code
#          inside  the docker container
# volumes:
#   - ../../../../:/workspace/Sming
   
 ports:
 #choose a free port to connect to the web C9 editor
 - "10080:80"
 
 devices:
  # uncomment to map your serial USB adaptor 
  #- "/dev/ttyUSB0:/dev/ttyUSB0"

```

## Start your container

```shell
docker-compose up -d 
```
## Open your browser

http://localhost:10080

[![C9 screenshot example](https://raw.githubusercontent.com/wiki/SmingHub/Sming/images/c9-1.png)](images/c9-1.png)

And inside of the Terminal tab in your browser window you can type

```shell
cd $SMING_HOME
make
```

```shell
cd $SMING_HOME/../samples/Basic_Blink
make
make flash
```

## Shut down your container

```shell
docker-compose down
```
