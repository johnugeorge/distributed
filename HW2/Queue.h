#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <queue>


template <typename T>
class Queue
{
	public:

		Queue() { }

		int putq(const T &aObj) {
			pthread_mutex_lock(&lock);
			mRep.push(aObj);

			pthread_mutex_unlock(&lock);
			return 0;
		}

		int getq(T &aObj) {
			pthread_mutex_lock(&lock);

			if (mRep.empty())
			{
			 	pthread_mutex_unlock(&lock);
				return -1;
			}

			aObj = mRep.front();
			mRep.pop();

			pthread_mutex_unlock(&lock);
			return 0;
		}

		bool empty(){
			pthread_mutex_lock(&lock);
			if(mRep.empty())
			{
			        pthread_mutex_unlock(&lock);
				return true;
			}
			else
			{
			        pthread_mutex_unlock(&lock);
				return false;
			}
		}
		int size(){
			return mRep.size();
		}
	private:
		pthread_mutex_t lock;
		std::queue<T> mRep;

};

#endif // _QUEUE_H_

