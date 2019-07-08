#!/bin/bash

#Post processing time parameters 
step="1.0" #s
startTime="5.0" #s
endTime="105.0" #s

step_ms="1000" #ms
startTime_ms="5000" #ms
endTime_ms="105000" #ms

#Compiling
./waf

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/build/lib

#Varying parameters
relayUeInitXPos="300"
remoteUeInitXPosStart=100
remoteUeInitXPosEnd=1650
remoteUeInitXPosStep=50

useRelayArray="true false"
#useRelayArray="true"

#for useRelay in $useRelayArray
#do 
function RunSim {
    useRelay=$1

  echo '' > "output_wns3-2019-relay_useRelay_${useRelay}_UlAvgResourceUtilizationPerRemoteUeInitXPos.txt"
  echo '' > "output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgAppThroughputPerRemoteUeInitXPos.txt"
  echo '' > "output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgDelayPerRemoteUeInitXPos.txt"

  remoteUeInitXPos=$remoteUeInitXPosStart
  while [  $remoteUeInitXPos -lt $remoteUeInitXPosEnd ]; do

    outputDir="output_wns3-2019-relay_useRelay_${useRelay}_remoteUeInitXPos_${remoteUeInitXPos}"
    mkdir -p $outputDir
    cd $outputDir

    ###########################################################################
    #Running the simulations
    ###########################################################################

    ../build/scratch/wns3-2019-relay --useRelay=$useRelay --remoteUeInitXPos=$remoteUeInitXPos --relayUeInitXPos=$relayUeInitXPos
    #../build/scratch/wns3-2019-relay --useRelay=$useRelay --remoteUeInitXPos=$remoteUeInitXPos --relayUeInitXPos=$relayUeInitXPos --RngRun=2

    ###########################################################################
    #Calculating statistics
    ###########################################################################

    ###############################UPWARD######################################
    #Upward APP Throughput

    #Get the Upward Txs Node IDs
    UpwTxNodeIds=`awk '{ if ($2 == "tx") {txters[$3]=1;} }END {for (i in txters){print i;}}' AppPacketTrace_Upward.txt | sort -n`

    UpwStringToPlotTput=''
    UpwstringToPlotDelay=''
    for upwTxNodeId in $UpwTxNodeIds
    do 
      
      #Get only the packets txted by upwTxNodeId and succesfully received at the other end
      awk -v upwTxNodeId=$upwTxNodeId '{if($2 == "tx" && $3 == upwTxNodeId) {PcktsTxByThisNode[$6] = 1; } if ($2 == "rx" && PcktsTxByThisNode[$6] == 1 ) {print $0} }' "AppPacketTrace_Upward.txt" > "AppPacketTrace_Upward_TxNode_$upwTxNodeId.txt"

      #Calculate Throughput over time for upwTxNodeId
      awk -v step=$step -v startTime=$startTime -v endTime=$endTime 'BEGIN { OFMT = "%.10f"; CONVFMT="%.10f"; minT=startTime; maxT=endTime}
        {
          rxBytesByTime[$1] += $5;
        } 
        END{
          if (FNR==1) {print "0","0"; exit;}
          for (t=minT; t<=maxT; t+=step)
          {
            for (i in rxBytesByTime) 
            { 
              i_num=i+0;
              if (i_num >= t && i_num < t + step)
              {
                 timelineBytes[t]+= rxBytesByTime[i]; 
              } 
            }
            print t, (timelineBytes[t]/(step));
          } 
        }' "AppPacketTrace_Upward_TxNode_$upwTxNodeId.txt" | sort -n > "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_ThroughputOverTime.txt"

      UpwStringToPlotTput="$UpwStringToPlotTput \"AppPacketTrace_Upward_TxNode_${upwTxNodeId}_ThroughputOverTime.txt\" using 1:2 with linespoints title \"Node ${upwTxNodeId}\","

      #Calculate Packet Delay of the packets txted by upwTxNodeId and succesfully received at the other end
      awk -v upwTxNodeId=$upwTxNodeId 'BEGIN { OFMT = "%.10f"; CONVFMT="%.10f"; nRxPackets=0;} 
        {
          if($2 == "tx" && $3 == upwTxNodeId){
            TxTimePcktsTxByThisNode[$6] = $1;
          }
          if ($2 == "rx" && TxTimePcktsTxByThisNode[$6] != 0){
            delay[$6] = $1 - TxTimePcktsTxByThisNode[$6];
            print $1, $6, delay[$6];
            nRxPackets++;
          }  
        }
        END{
          if (nRxPackets<1) {print "0","0","0";} 
        }' "AppPacketTrace_Upward.txt" | sort -n > "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_RxPacketDelayOverTime.txt"

      UpwstringToPlotDelay="$UpwstringToPlotDelay \"AppPacketTrace_Upward_TxNode_${upwTxNodeId}_RxPacketDelayOverTime.txt\" using 1:3 with linespoints title \"Node ${upwTxNodeId}\","


      #Averages
      #Throughput
      awk -v startTime=$startTime -v endTime=$endTime -v step=$step 'BEGIN{sum=0; count=0; sumForStd=0; }
      {if ($1>=startTime && $1 < (endTime+step) ) {
         sum+=$2; count++; values[count]=$2;}
      }END{
        if (count > 0 ){
          mean=sum/count; 
          for (i in values) {
            delta=values[i]-mean; 
            sumForStd+=(delta^2); 
          }
          stdev = sqrt(sumForStd/count);
          ci = 1.96 * (stdev/(sqrt (count)));
          #print mean, stdev;
          print mean, ci;
        } 
        else {
          print 0, 0;
        }
      }' "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_ThroughputOverTime.txt" > "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_ThroughputAverage.txt"

      awk -v remoteUeInitXPos=$remoteUeInitXPos -v upwTxNodeId=$upwTxNodeId '{print remoteUeInitXPos, upwTxNodeId, $0;}' "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_ThroughputAverage.txt" >> "../output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgAppThroughputPerRemoteUeInitXPos.txt"

      #Delay
      awk -v startTime=$startTime -v endTime=$endTime  -v step=$step 'BEGIN{sum=0; count=0; sumForStd=0; }
      {if ($1>=startTime && $1 < (endTime+step) ) {
         sum+=$3; count++; values[count]=$3;}
      }END{
        if (count > 0 ){
          mean=sum/count; 
          for (i in values) {
            delta=values[i]-mean; 
            sumForStd+=(delta^2); 
          }
          stdev = sqrt(sumForStd/count); 
          ci = 1.96 * (stdev/(sqrt (count)));
          #print mean, stdev;
          print mean, ci;
        } 
        else {
          print 0, 0;
        }
      }' "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_RxPacketDelayOverTime.txt" > "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_RxPacketDelayAverage.txt"
      
      awk -v remoteUeInitXPos=$remoteUeInitXPos -v upwTxNodeId=$upwTxNodeId '{print remoteUeInitXPos, upwTxNodeId, $0;}' "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_RxPacketDelayAverage.txt" >> "../output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgDelayPerRemoteUeInitXPos.txt"

    done

    echo "reset 
          #set terminal pngcairo nocrop enhanced size 1280,1024
          set terminal pngcairo nocrop enhanced size 800,600
          set output \"AppPacketTrace_Upward_ThroughputOverTime.png\"
          set title \"App Throughput per Tx Node - (Bytes/s) \n Upward Direction \n Sampling window size = $step s \"
          set yrange [0:]	
          set xlabel \"Time (s)\"
          set ylabel \"Throughput (Bytes/s)\"
          set key outside right center
          plot $UpwStringToPlotTput" | gnuplot


    echo "reset 
          #set terminal pngcairo nocrop enhanced size 1280,1024
          set terminal pngcairo nocrop enhanced size 800,600
          set output \"AppPacketTrace_Upward_RxPacketDelayOverTime.png\"
          set title \"App Packet Delay per Tx Node - (s) \n Upward Direction \n t = packet reception time \"
          set yrange [0:]	
          set xlabel \"Time (s)\"
          set ylabel \"Delay (s)\"
          set key outside right center
          plot $UpwstringToPlotDelay" | gnuplot



    #Cleaning
    for upwTxNodeId in $UpwTxNodeIds
    do 
      rm "AppPacketTrace_Upward_TxNode_$upwTxNodeId.txt"
      rm "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_ThroughputOverTime.txt"
      rm "AppPacketTrace_Upward_TxNode_${upwTxNodeId}_RxPacketDelayOverTime.txt"
    done


    #UL Resource utilization (UL + SL)
    if [ -f "UlRxPhyStats.txt" ]
    then
      tail -n +2 UlRxPhyStats.txt > RxPhyStatsToUse.txt

      if [ -f "SlRxPhyStats.txt" ]
      then

        remoteImsi="2" 
        awk -v remoteImsi=$remoteImsi '{if ($3 != remoteImsi) {print $0} } ' "SlRxPhyStats.txt" >> "RxPhyStatsToUse.txt"
        #tail -n +2 SlRxPhyStats.txt >> RxPhyStatsToUse.txt
      fi

      sort -n RxPhyStatsToUse.txt > RxPhyStatsToUse_sorted.txt

      awk -v step_ms=$step_ms -v startTime_ms=$startTime_ms -v endTime_ms=$endTime_ms 'BEGIN { OFMT = "%.10f"; CONVFMT="%.10f"; minT=startTime_ms; maxT=endTime_ms; bw=50}
          {
            rbsByTime[$1] += $7;
          } 
          END{
            if (FNR==1) {print "0","0"; exit;}
            for (t=minT; t<=maxT; t+=step_ms)
            {
              for (i in rbsByTime) 
              { 
                i_num=i+0;
                if (i_num >= t && i_num < t + step_ms)
                {
                   timelineRbs[t]+= rbsByTime[i]; 
                } 
              }
              print t, (timelineRbs[t]/(step_ms*bw));
            } 
          }'  RxPhyStatsToUse_sorted.txt | sort -n > "UL_ResourceUtilizationOverTime.txt"

      echo "reset 
            #set terminal pngcairo nocrop enhanced size 1280,1024
            set terminal pngcairo nocrop enhanced size 800,600
            set output \"UL_ResourceUtilizationOverTime.png\"
            set title \"UL Resource Utilization \n Sampling window size = $step_ms ms\"
            set yrange [0:]	
            set xlabel \"Time (ms)\"
            set ylabel \"Resource utilization\"
            unset key
            plot 'UL_ResourceUtilizationOverTime.txt' using 1:2 with linespoints " | gnuplot

      #Average 
      awk -v startTime=$startTime_ms -v endTime=$endTime_ms  -v step=$step_ms 'BEGIN{sum=0; count=0; sumForStd=0; }
      {if ($1>=startTime && $1 < (endTime+step) ) {
         sum+=$2; count++; values[count]=$2;}
      }END{
        if (count > 0 ){
          mean=sum/count; 
          for (i in values) {
            delta=values[i]-mean; 
            sumForStd+=(delta^2); 
          }
          stdev = sqrt(sumForStd/count); 
          ci = 1.96 * (stdev/(sqrt (count)));
          #print mean, stdev;
          print mean, ci;
        } 
        else {
          print 0, 0;
        }
      }' UL_ResourceUtilizationOverTime.txt > UL_ResourceUtilizationOverTime_Average.txt
      awk -v remoteUeInitXPos=$remoteUeInitXPos '{print remoteUeInitXPos, $0;}' UL_ResourceUtilizationOverTime_Average.txt >> "../output_wns3-2019-relay_useRelay_${useRelay}_UlAvgResourceUtilizationPerRemoteUeInitXPos.txt"
      
      #Cleaning
      rm RxPhyStatsToUse.txt
      rm RxPhyStatsToUse_sorted.txt

    else
      echo -e "$remoteUeInitXPos\t0" >> "../output_wns3-2019-relay_useRelay_${useRelay}_UlAvgResourceUtilizationPerRemoteUeInitXPos.txt"
    fi

    ###############################################################################

    ###############################DOWNWARD########################################

    #Downward APP Throughput

    #Get the Upward Txs Node IDs
    DwdRxNodeIds=`awk '{ if ($2 == "rx") {rcvers[$3]=1;} }END {for (i in rcvers){print i;}}' AppPacketTrace_Downward.txt`

    DwdStringToPlotTput=''
    DwdStringToPlotDelay=''
    for dwdRxNodeId in $DwdRxNodeIds
    do 

      #We don't have the info about the transmissions in the downward (UdpEchoServer does not have the trace source)
      #Thus, we can only calculate troughput
      #Get only the packets succesfully rxted by dwdRxNodeId 
      awk -v dwdRxNodeId=$dwdRxNodeId '{if($2 == "rx" && $3 == dwdRxNodeId) {print $0} }' "AppPacketTrace_Downward.txt" > "AppPacketTrace_Downward_RxNode_$dwdRxNodeId.txt"
     

     #Calculate Throughput over time for dwdRxNodeId
      awk -v step=$step -v startTime=$startTime -v endTime=$endTime  'BEGIN { CONVFMT="%.10f"; minT=startTime; maxT=endTime}
        {
          rxBytesByTime[$1] += $5;
          #if (NR==1) {minT = $1;} if (NR==FNR){maxT=$1} 
        } 
        END{

          if (FNR==1) {print "0","0"; exit;}
          for (t=minT; t<=maxT; t+=step)
          {
            for (i in rxBytesByTime) 
            { 
              i_num=i+0;
              if ((i_num >= t) && (i_num < (t + step)))
              {
                 timelineBytes[t]+= rxBytesByTime[i]; 
              } 
            }
            print t, (timelineBytes[t]/(step));
          } 
        }' "AppPacketTrace_Downward_RxNode_$dwdRxNodeId.txt" > "AppPacketTrace_Downward_RxNode_${dwdRxNodeId}_ThroughputOverTime.txt"

      DwdStringToPlotTput="$DwdStringToPlotTput \"AppPacketTrace_Downward_RxNode_${dwdRxNodeId}_ThroughputOverTime.txt\" using 1:2 with linespoints title \"Node ${dwdRxNodeId}\","

      #Calculate Packet Delay of the packets txted by upwTxNodeId and succesfully received at the other end
      awk  -v dwdRxNodeId=$dwdRxNodeId 'BEGIN { OFMT = "%.10f"; CONVFMT="%.10f"; nRxPackets=0;} 
        {
          if($2 == "tx" ){
            TxTimePcktsTx[$6] = $1;
          }
          if ($2 == "rx" && $3 == dwdRxNodeId && TxTimePcktsTx[$6] != 0){
            delay[$6] = $1 - TxTimePcktsTx[$6];
            print $1, $6, delay[$6];
            nRxPackets ++;
          }  
        }END{
          if (nRxPackets < 1) {print 0, 0, 0;}
        }' "AppPacketTrace_Downward.txt" | sort -n > "AppPacketTrace_Downward_RxNode_${dwdRxNodeId}_RxPacketDelayOverTime.txt"

      DwdStringToPlotDelay="$DwdStringToPlotDelay \"AppPacketTrace_Downward_RxNode_${dwdRxNodeId}_RxPacketDelayOverTime.txt\" using 1:3 with linespoints title \"Node ${dwdRxNodeId}\","

    done

    if [ !  -z  "$DwdStringToPlotDelay" ]
    then
      echo "reset 
            #set terminal pngcairo nocrop enhanced size 1280,1024
            set terminal pngcairo nocrop enhanced size 800,600
            set output \"AppPacketTrace_Downward_ThroughputOverTime.png\"
            set title \"App Throughput per Rx Node - (Bytes/s) \n Downward Direction \n Sampling window size = $step s \"
            set yrange [0:]	
            set xlabel \"Time (s)\"
            set ylabel \"Throughput (Bytes/s)\"
            set key outside right center
            plot $DwdStringToPlotTput" | gnuplot

      echo "reset 
            #set terminal pngcairo nocrop enhanced size 1280,1024
            set terminal pngcairo nocrop enhanced size 800,600
            set output \"AppPacketTrace_Downward_RxPacketDelayOverTime.png\"
            set title \"App Packet Delay per Rx Node - (s) \n Downward Direction \n t = packet reception time \"
            set yrange [0:]	
            set xlabel \"Time (s)\"
            set ylabel \"Delay (s)\"
            set key outside right center
            plot $DwdStringToPlotDelay" | gnuplot
    fi


    #Cleaning
    for dwdRxNodeId in $DwdRxNodeIds
    do 
      rm "AppPacketTrace_Downward_RxNode_$dwdRxNodeId.txt"
      rm "AppPacketTrace_Downward_RxNode_${dwdRxNodeId}_ThroughputOverTime.txt"
      rm "AppPacketTrace_Downward_RxNode_${dwdRxNodeId}_RxPacketDelayOverTime.txt"
    done

    cd .. #Important!

    let remoteUeInitXPos=remoteUeInitXPos+$remoteUeInitXPosStep

  done   #while [  $remoteUeInitXPos -lt $remoteUeInitXPosEnd ]

  
  if [ "$useRelay" = "true" ]
  then
    linecolor="1"
  else
    linecolor="3"
  fi

  for upwTxNodeId in $UpwTxNodeIds
  do 
    awk -v upwTxNodeId=$upwTxNodeId '{if ($2 == upwTxNodeId){print $0;}}' "output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgAppThroughputPerRemoteUeInitXPos.txt" | sort -n > output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgAppThroughputPerRemoteUeInitXPos_${upwTxNodeId}.txt

    awk -v upwTxNodeId=$upwTxNodeId '{if ($2 == upwTxNodeId && $3 != 0){print $0;}}' "output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgDelayPerRemoteUeInitXPos.txt" > "output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgDelayPerRemoteUeInitXPos_${upwTxNodeId}.txt"

  done

}

UlStringToPlotResUtil=""
UwdStringToPlotAvgTput=""
UwdStringToPlotAvgDelay=""
upwTxNodeId=4

for useRelay in $useRelayArray
do
    RunSim $useRelay &
    if [ "$useRelay" = "true" ]
    then
	    legend="With relay"
	    linecolor="1"
    else
	    legend="Without relay"
	    linecolor="3"
    fi
    UlStringToPlotResUtil="$UlStringToPlotResUtil \"output_wns3-2019-relay_useRelay_${useRelay}_UlAvgResourceUtilizationPerRemoteUeInitXPos.txt\" using 1:(100*\$2):3 with yerr title \"${legend}\", '' using 1:(100*\$2) with lines lc $linecolor notitle,"
    UwdStringToPlotAvgTput="$UwdStringToPlotAvgTput \"output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgAppThroughputPerRemoteUeInitXPos_${upwTxNodeId}.txt\" using 1:3:4 with yerr title \"${legend}\", '' using 1:3 with lines lc $linecolor notitle,"
    UwdStringToPlotAvgDelay="$UwdStringToPlotAvgDelay \"output_wns3-2019-relay_useRelay_${useRelay}_UpwardAvgDelayPerRemoteUeInitXPos_${upwTxNodeId}.txt\" using 1:3:4 with yerr title \"${legend}\", '' using 1:3 with lines lc $linecolor notitle,"
    
done #for useRelay in $useRelayArray

echo "Waiting for simulations to complete"
wait
echo "Done"

echo "reset 
      set terminal pngcairo nocrop enhanced size 800,600
      set output \"output_wns3-2019-relay_UL_ResourceUtilizationPerRemoteUeInitXPos.png\"
      #set terminal postscript eps enhanced color font 'Helvetica,20'
      #set output \"utilization.eps\"
      #set title \"UL Average Resource Utilization \n Sampling window size = $step s\"
      set yrange [0:0.8]	
      set xlabel \"Distance between remote UE and eNodeB (m)\"
      set ylabel \"Uplink resource utilization (%)\"
      set key inside right top
      plot $UlStringToPlotResUtil " | gnuplot
    
echo "reset 
      set terminal pngcairo nocrop enhanced size 800,600
      set output \"output_wns3-2019-relay_Upward_ThroughputPerRemoteUeInitXPos.png\"
      #set terminal postscript eps enhanced color font 'Helvetica,20'
      #set output \"throughput.eps\"
      #set title \"Average App Throughput per Tx Node - (Bytes/s) \n Upward Direction \n Sampling window size = $step s \"
      set yrange [0:1000]	
      set xlabel \"Distance between remote UE and eNodeB (m)\"
      set ylabel \"Throughput (Bytes/s)\"
      set key inside right top
      plot $UwdStringToPlotAvgTput" | gnuplot

echo "reset 
      set terminal pngcairo nocrop enhanced size 800,600
      set output \"output_wns3-2019-relay_Upward_DelayPerRemoteUeInitXPos.png\"
      #set terminal postscript eps enhanced color font 'Helvetica,20'
      #set output \"delay.eps\"
      #set title \"Average Packet Delay per Tx Node - (s) \n Upward Direction \n Sampling window size = $step s \"
      set yrange [0:0.14]	
      set xlabel \"Distance between remote UE and eNodeB (m)\"
      set ylabel \"Delay (s)\"
      set key inside right top
      plot $UwdStringToPlotAvgDelay" | gnuplot



