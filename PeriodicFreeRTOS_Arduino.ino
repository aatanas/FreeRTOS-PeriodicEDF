#include <Arduino_FreeRTOS.h>
#define BUFF_SIZE 16
#define TSK0 "0_ON"
#define TSK1 "1_OFF"
#define PER0 100 //2000 / portTICK_PERIOD_MS(16) 125
#define PER1 300 //5000 / portTICK_PERIOD_MS(16) 312

#define printff(args...) printf(args); fflush_SERIAL()
#define printlnf(x) println(x); Serial.flush()

typedef struct task_params{
	char pcName[configMAX_TASK_NAME_LEN];
	TickType_t uxPeriod;
	unsigned char fp;
	unsigned int uRTParam;
} task_params;

static task_params myBatch[configMAX_EDF_TASKS];
static TickType_t pxRTParams[configMAX_EDF_TASKS];
static unsigned char n;
static char read_buff[BUFF_SIZE];
static float RunTimeSum;
static float newRunTimeSum;
//static TickType_t x;

int serial_putchar(char c, FILE* f) {
   if (c == '\n') serial_putchar('\r', f);
   return Serial.write(c) == 1? 0 : 1;
}

FILE serial_stdout;

void TaskBlink0( void *pvParameters );
void TaskBlink1( void *pvParameters );

void (*func_ptr[2])(void *) = {TaskBlink0, TaskBlink1};

void setup() {
	Serial.begin(115200); //115200
	while (!Serial);
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);
	fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
	stdout = &serial_stdout;
	stderr = stdout;
	Serial.printlnf(F("Hello from setup()!"));
}

void loop()
{
	/*if(!x){
		Serial.print(F("Hello from loop()! "));
		x++;
		printf("%u\n",xTaskGetTickCount());
	}
	else{
		TickType_t ticks = xTaskGetTickCount();
		if(ticks % (PER1) == 0 && ticks != x){
			Serial.print(F("Hello again from loop! "));
			x = ticks;
			printf("%u\n",x);
		}
	}*/
	if(Serial.available()){ //portDISABLE_INTERRUPTS() ?
		while(Serial.available()){
			unsigned char c = Serial.read();
			if(c =='@'){
				xSetEDFPending(pdTRUE);
				c = Serial.readBytesUntil('|', read_buff, BUFF_SIZE);
				read_buff[c]=0;
				sscanf(read_buff,"%s %hhu %u %u",myBatch[n].pcName, &(myBatch[n].fp), &(myBatch[n].uxPeriod), &(myBatch[n].uRTParam));
				//@0_tsk 0 125 0|@1_tsk 1 312 0|

				if(!n) newRunTimeSum = RunTimeSum;
				float taskRatio = (float)myBatch[n].uRTParam / (float) myBatch[n].uxPeriod;
				if((newRunTimeSum += taskRatio) > 1 ){
					Serial.println(F("Nemoguc raspored!"));
					while(Serial.available()) Serial.read();
					n=0;
				}
				else n++;
			}
			else if (c == '!'){
				c = Serial.readBytesUntil('|', read_buff, BUFF_SIZE);
				vTaskRemoveEDF(read_buff[0]);
			}
		}
		if(n){
			for(int i=0;i<n;i++){
				pxRTParams[myBatch[i].pcName[0]-'0'] = myBatch[i].uRTParam;
				xTaskCreateEDF(func_ptr[myBatch[i].fp], myBatch[i].pcName, pxRTParams+(myBatch[i].pcName[0]-'0'), myBatch[i].uxPeriod);
			}
			RunTimeSum = newRunTimeSum;
			n=0;
		}
		xSetEDFPending(pdFALSE);
	}

}

void TaskBlink0(void *pvParameters)
{
	TickType_t RTParam = *((TickType_t*)pvParameters);
	Serial.print(F("Hello form task0!")); printf(" |%u\n",xTaskGetTickCount());
	digitalWrite(LED_BUILTIN, HIGH);
	TickType_t startCount = xTaskGetTickCount();
	while(xTaskGetTickCount() < startCount + RTParam);
	Serial.print(F("Goodbye form task0")); printf(" |%u\n",xTaskGetTickCount());
	vTaskEndEDF(-1);
}

void TaskBlink1(void *pvParameters)
{
	TickType_t RTParam = *((TickType_t*)pvParameters);
	Serial.print(F("Hello form task1")); printf(" |%u\n",xTaskGetTickCount());
	digitalWrite(LED_BUILTIN, LOW);
	vTaskDelay(RTParam);
	Serial.print(F("Goodbye form task1")); printf(" |%u\n",xTaskGetTickCount());
	vTaskEndEDF(-1);
}

