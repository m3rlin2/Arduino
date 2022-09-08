#!/bin/zsh

### Default values

broker="localhost"
port=1883
topic="/home/living/dht/cmd/update"
outFile=""
inFile=""
splitSize=500
keep="false"
user=""
pw=""

### Eval input arguments

while getopts ":hb:p:U:P:t:s:o:k" opt; do
	case $opt in
		h)
			echo "Usage of $0:"
			echo "$0 [-b mqtt broker] [-p mqtt port] [-U mqtt user] [-P mqtt pw] [-t topic] [-s packet size] [-o outputFile] [-k] inputFile"
			echo "-k: \t keep old file as *.old"
			exit 1
			;;
		b)
			broker=$OPTARG
			;;
		p)
			port=$OPTARG
			;;
		U)
			user=$OPTARG
			;;
		P)
			pw=$OPTARG
			;;
		t)
			topic=$OPTARG
			;;
		s)
			splitSize=$OPTARG
			;;
		o)
			outFile=$OPTARG
			;;
		k)
			keep="true"
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit 1
			;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1
			;;
	esac
done
shift $(expr $OPTIND - 1 )

### Begin of checking

if (( $# == 0 )); then
	echo "No input file specified. See help with '$0 -h'" >&2
	exit 1
elif (( $# > 1 )); then
	echo "Only one input file allowed. See help with '$0 -h'" >&2
	exit 1
fi

inFile=$1
if [ ! -f $inFile ]; then
    echo "Input file not found or no regular file. See help with '$0 -h'" >&2
	exit 1
fi

if ((${#outFile} == 0)); then
	outFile=$(basename $inFile)
fi

### End of checking

pre="/tmp/${outFile}_splitted_"

split -d -b $splitSize $inFile $pre
i=1
ls -1 $pre* | sort -n | while read line
do
	parts[i]=$line
	((i++))
done

n=0
N=${#parts[@]}
for tmpFile in ${parts[@]}
do
	((n++))
	start="$n/$N#$outFile#$keep#"
	echo "$inFile: Sending part $n/$N - $parts[n]"
	if ((${#user} == 0)); then
		mosquitto_pub -h $broker -p $port -t $topic -m "$start$(cat $tmpFile)"
	else
		mosquitto_pub -h $broker -p $port -u $user -P $pw -t $topic -m "$start$(cat $tmpFile)"
	fi
	sleep 0.5s
	rm $tmpFile
	echo "$inFile: Removed part $n/$N - $parts[n]"
done

exit 0
