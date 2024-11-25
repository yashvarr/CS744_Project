1. make
2. chmod +x loadgen.sh
3. ./broker
4. ./loadgen n {n = number of subs}
      - this will generate random subs and assign three different topics
5. ./publisher
   > Enter Topic - news (for eg)
   > Enter Message - Hi from news
6. ^C in loadgen will close all the subs - connection

7. The message will be published to all the subscribers subscribed to that topic.
