#include "TaskQueue.h"
#include "pthread.h"
#include "logger.h"

static void* runTask(void* args) {
	TaskQueue* taskQueue = static_cast<TaskQueue*>(args);
	if (nullptr == taskQueue)
		return (void*)-1;

	taskQueue->DoTask();
	
	log_info("{%s:%s:%d} runTask Thread exit ",
		__FILE__, __FUNCTION__, __LINE__);

	return (void*)0;
}

void TaskQueue::DoTask() {

	while (!m_bStop) {
		
		int nSize = GetTaskNo();
		if (nSize <= 0) {
			MSleep(5);
			continue;
		}

		m_cMutex.Lock();
		Task* uploadTask = (Task*)m_dQueue.front();
		m_cMutex.Unlock();

		if (nullptr == uploadTask) {
			continue;
		}

		std::cout << "fetch task to do "<< uploadTask->taskDescr() 
			<< std::endl;
		
		uploadTask->setQueue(this);
		uploadTask->setExecStart(time(0));
		int nResult = uploadTask->Run();
		uploadTask->setExecEnd(time(0));
		if (0 == nResult) {
			
			m_cMutex.Lock();
			m_dQueue.pop_front();
			uploadTask->FinishCb();
			m_cMutex.Unlock();
			//delete uploadTask;
		}
		else {
			m_cMutex.Lock();
			m_dQueue.pop_front();
			m_dQueue.push_back(uploadTask);// reenter queue
			m_cMutex.Unlock();
		}
		
		MSleep(2);
	}
}


void TaskQueue::Start() {
	m_bStop = false;
	
	log_info("{%s:%s:%d} TaskQueue Start ",
		__FILE__, __FUNCTION__, __LINE__);

	pthread_create(&m_uploadTask_t, NULL, runTask, (void*)this);

}

void TaskQueue::Stop() {
	m_bStop = true;
	
	MSleep(20);//

	log_info("{%s:%s:%d} TaskQueue Stop ",
		__FILE__, __FUNCTION__, __LINE__);

	pthread_join(m_uploadTask_t, NULL);

}