#ifndef PCX_MESSAGE_BUS_H
#define PCX_MESSAGE_BUS_H

#include <list>
#include <unordered_map>
#include <functional>
#include <typeindex>

namespace pcx
{
   /**
    * @brief The MessageBus class is used to send cross-component messages.
    * Users subscribe with a simple callback, and are notified of messages
    * (and the sender of that message) as a result of invocation of the
    * publish method
    */
   class MessageBus {
   public:
      template <typename Message>
      void subscribe(std::function<void(void*,Message const &)> callback) {
         auto it = publishers_.find(typeid(Message));
         if (it == publishers_.end()) {
            publishers_[typeid(Message)] = new Publisher<Message>{};
            it = publishers_.find(typeid(Message));
         }
         auto & publisher = *reinterpret_cast<Publisher<Message>*>(it->second);
         publisher.subscribers.push_back(callback);
      }

      template <typename Message>
      void publish(void* sender, Message const & message) {
         auto it = publishers_.find(typeid(Message));
         if (it == publishers_.end()) return;

         auto & publisher = *reinterpret_cast<Publisher<Message>*>(it->second);
         publisher.publish(sender, message);
      }

   private:
      template <typename Message>
      struct Publisher {
         std::list<std::function<void(void*,Message const &)>> subscribers;

         void publish(void* sender, Message const & message) {
            for (auto & sub : subscribers) sub(sender, message);
         }
      };

      std::unordered_map<std::type_index, void*> publishers_;
   };

} // namespace pcx

#endif // #ifndef PCX_MESSAGE_BUS_H
