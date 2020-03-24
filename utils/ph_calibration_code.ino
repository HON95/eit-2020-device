#define PH_PORT A3

#define PH_OFFSET -1.03
#define PH_GRADIENT 4.183

void setup() {
  pinMode(PH_PORT, INPUT);
  Serial.begin(9600);

}

void loop() {

  float ph = measurePH();
  Serial.print("Average: ");
  Serial.println(ph);

  delay(15000);

}

int sort_cmp(const void *cmp1, const void *cmp2)
{
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  
  return a > b ? -1 : (a < b ? 1 : 0);
}

int aggregateSamples(int samples[]){
  qsort(samples, sizeof(samples)/sizeof(samples[0]), sizeof(samples[0]), sort_cmp);

  int sampleSum = 0;
  for(int i = 2; i < 8; i++){
    sampleSum += samples[i];
  }

  return sampleSum;
}

float measurePH(){

  int samples[10] = {};
  for(int i = 0; i < 10; i++){
    int r = analogRead(PH_PORT);
    Serial.println(r);
    samples[i] = r;
    delay(5000);
  }

  int sampleSum = aggregateSamples(samples);

  return PH_OFFSET + (PH_GRADIENT * (float)sampleSum * 5.0) / (1024 * 6);
}
