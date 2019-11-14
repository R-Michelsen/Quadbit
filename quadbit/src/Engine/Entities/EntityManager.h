#pragma once
#include <deque>
#include <execution>
#include <any>

#include "Engine/Entities/QbEntityDefs.h"
#include "Engine/Entities/SystemDispatch.h"
#include "Engine/Entities/SparseSet.h"
#include "Engine/Rendering/RenderTypes.h"

namespace Quadbit {
	class EntityManager;
	struct Entity {
		EntityID id_;

		Entity();
		Entity(EntityID id) : id_(id) {}

		bool operator == (const Entity& other) const { return (id_ == other.id_); }
		bool operator != (const Entity& other) const { return (id_ != other.id_); }

		void Destroy();
		bool IsValid();

		template<typename T>
		void AddComponent();

		// Aggregate initialization
		template<typename T>
		void AddComponent(T&& t);

		template<typename... T>
		void AddComponents();

		template<typename T>
		T* const GetComponentPtr();

		template<typename T>
		bool HasComponent();

		template<typename T>
		void RemoveComponent();
	};

	const Entity NULL_ENTITY{ {0xFFFF'FFFF, 0xFFFF'FFFF} };

	struct EntityDestroyCommand {
		Entity entity;

		void Play() {
			entity.Destroy();
		}
	};

	struct EntityCommandBuffer {
	public:
		std::vector<EntityDestroyCommand> destroyBuffer_;

		void DestroyEntity(Entity entity) {
			EntityDestroyCommand destroyCmd;
			destroyCmd.entity = entity;
			destroyBuffer_.push_back(destroyCmd);
		}

		void PlayCommands() {
			for (auto&& cmd : destroyBuffer_) {
				cmd.Play();
			}
		}
	};

	struct ComponentPool {
		void* sparseSet;
		void(*RemoveIfExists)(void*, EntityID id);
		void(*Destructor)(void*);

		template<typename C>
		static ComponentPool New() {
			return ComponentPool {
				new SparseSet<C>(),
				[](void* sparseSet, EntityID id) { static_cast<SparseSet<C>*>(sparseSet)->RemoveIfExists(id); },
				[](void* sparseSet) { static_cast<const SparseSet<C>*>(sparseSet)->~SparseSet<C>(); }
			};
		}

		template<typename C>
		SparseSet<C>* GetPtr() {
			return reinterpret_cast<SparseSet<C>*>(sparseSet);
		}
	};

	/*
	Note on entity manager behaviour:
	On release builds registering a component twice, adding a component twice
	or removing a non-existent component results in undefined behaviour
	In debug-mode asserts should catch this
	*/
	class EntityManager {
	public:
		std::unique_ptr<SystemDispatch> systemDispatch_;
		std::array<ComponentPool, MAX_COMPONENTS> componentPools_;

		static EntityManager& Instance();

		Entity Create();
		void Destroy(const Entity& entity);
		bool IsValid(const Entity& entity);

		void Shutdown() {
			systemDispatch_->Shutdown();
			systemDispatch_.reset();
			for (auto&& component : componentPools_) {
				// We can break at the first null-pointer since component pools
				// cannot be unregistered (destroyed) at runtime and thus when we
				// encounter a nullptr, no pools are left in the array.
				if (component.sparseSet != nullptr) break;
				component.Destructor(component.sparseSet);
			}
		}

		template<typename C>
		SparseSet<C>* GetComponentStoragePtr() {
			size_t componentID = ComponentID::GetUnique<C>();
			return reinterpret_cast<SparseSet<C>*>(componentPools_[componentID].sparseSet);
		}

		template<typename C>
		void RegisterComponent() {
			size_t componentID = ComponentID::GetUnique<C>();
			auto componentStorage = Instance().GetComponentStoragePtr<C>();
			assert(componentStorage == nullptr && "Failed to register component: Component is already registered with the entity manager\n");
			componentPools_[componentID] = ComponentPool::New<C>();
		}

		template<typename... C>
		void RegisterComponents() {
			(RegisterComponent<C>(), ...);
		}

		template<typename C>
		void AddComponent(const Entity& entity) const {
			auto* componentStorage = Instance().GetComponentStoragePtr<C>();
			assert(componentStorage != nullptr && "Failed to add component: Component isn't registered with the entity manager\n");
			componentStorage->Insert(entity.id_);
		}

		// Aggregate initialization
		template<typename C>
		void AddComponent(const Entity& entity, C&& t) const {
			auto* componentStorage = Instance().GetComponentStoragePtr<C>();
			assert(componentStorage != nullptr && "Failed to add component: Component isn't registered with the entity manager\n");
			componentStorage->Insert(entity.id_, std::move(t));
		}

		template<typename C>
		C* const GetComponentPtr(const Entity& entity) const {
			return Instance().GetComponentStoragePtr<C>()->GetComponentPtr(entity.id_);
		}

		template<typename C>
		bool HasComponent(const Entity& entity) const {
			return Instance().GetComponentStoragePtr<C>()->HasComponent(entity.id_);
		}

		template<typename C>
		void RemoveComponent(const Entity& entity) const {
			auto* componentStorage = Instance().GetComponentStoragePtr<C>();
			assert(componentStorage != nullptr && "Failed to remove component: Component isn't registered with the entity manager\n");
			componentStorage->Remove(entity.id_);
		}

		template<typename C>
		using SparseSetPtr = SparseSet<C>*;

		template<typename... Components, typename F>
		void ForEach(F fun) {
			std::tuple pools{ GetPool<Components>()... };

			// Get the smallest pool to iterate from as a base
			std::vector<uint32_t> smallest = std::get<0>(pools)->entityFromComponentIndices_;
			ForEachTuple(pools, [&](auto&& value) {
				if (value->entityFromComponentIndices_.size() < smallest.size()) {
					smallest = value->entityFromComponentIndices_;
				}
				});

			for (auto entityIndex : smallest) {
				const bool allValid = ((std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex] != 0xFFFF'FFFF) && ...);
				if (!allValid) continue;

				fun(entities_[sparse_[entityIndex]], std::get<SparseSet<Components>*>(pools)->dense_[std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex]]...);

				(RemoveTag<Components>(entities_[sparse_[entityIndex]]), ...);
			}
		}

		template<typename... Components, typename F>
		void ForEachWithCommandBuffer(F fun) {
			std::tuple pools{ GetPool<Components>()... };

			// Get the smallest pool to iterate from as a base
			std::vector<uint32_t> smallest = std::get<0>(pools)->entityFromComponentIndices_;
			ForEachTuple(pools, [&](auto&& value) {
				if (value->entityFromComponentIndices_.size() < smallest.size()) {
					smallest = value->entityFromComponentIndices_;
				}
				});

			// Command buffer passed to the lambda, for recording commands that has to run after the for loop
			EntityCommandBuffer commandBuffer;

			for (auto entityIndex : smallest) {
				const bool allValid = ((std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex] != 0xFFFF'FFFF) && ...);
				if (!allValid) continue;

				fun(entities_[sparse_[entityIndex]], &commandBuffer, std::get<SparseSet<Components>*>(pools)->dense_[std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex]]...);

				(RemoveTag<Components>(entities_[sparse_[entityIndex]]), ...);
			}

			// Play back commands
			commandBuffer.PlayCommands();
		}

		template<typename... Components, typename F, typename T>
		void ForEachAddTag(F fun, T tag) {
			std::tuple pools{ GetPool<Components>()... };

			// Get the smallest pool to iterate from as a base
			std::vector<uint32_t> smallest = std::get<0>(pools)->entityFromComponentIndices_;
			ForEachTuple(pools, [&](auto&& value) {
				if (value->entityFromComponentIndices_.size() < smallest.size()) {
					smallest = value->entityFromComponentIndices_;
				}
				});

			for (auto entityIndex : smallest) {
				const bool allValid = ((std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex] != 0xFFFF'FFFF) && ...);
				if (!allValid) continue;

				fun(entities_[sparse_[entityIndex]], std::get<SparseSet<Components>*>(pools)->dense_[std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex]]...);

				(RemoveTag<Components>(entities_[sparse_[entityIndex]]), ...);
				AddTag<T>(entities_[sparse_[entityIndex]]);
			}
		}

		template<typename... Components, typename F>
		void ParForEach(F fun) {
			std::tuple pools{ GetPool<Components>()... };

			// Get the smallest pool to iterate from as a base
			std::vector<uint32_t> smallest = std::get<0>(pools)->entityFromComponentIndices_;
			ForEachTuple(pools, [&](auto&& value) {
				if (value->entityFromComponentIndices_.size() < smallest.size()) {
					smallest = value->entityFromComponentIndices_;
				}
				});

			// Command buffer passed to the lambda, for recording commands that has to run after the for loop
			EntityCommandBuffer commandBuffer;

			std::for_each(std::execution::par, smallest.begin(), smallest.end(), [&](auto entityIndex) {
				const bool allValid = ((std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex] != 0xFFFF'FFFF) && ...);
				if (!allValid) return;

				fun(entities_[sparse_[entityIndex]], &commandBuffer, std::get<SparseSet<Components>*>(pools)->dense_[std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex]]...);
				});

			// Remove tags, NOT SAFE IN PARRALEL
			std::for_each(std::execution::seq, smallest.begin(), smallest.end(), [&](auto entityIndex) {
				(RemoveTag<Components>(entities_[sparse_[entityIndex]]), ...);
				});

			// Play back commands
			commandBuffer.PlayCommands();
		}

		template<typename... Components, typename F>
		void ParForEachWithCommandBuffer(F fun) {
			std::tuple pools{ GetPool<Components>()... };

			// Get the smallest pool to iterate from as a base
			std::vector<uint32_t> smallest = std::get<0>(pools)->entityFromComponentIndices_;
			ForEachTuple(pools, [&](auto&& value) {
				if (value->entityFromComponentIndices_.size() < smallest.size()) {
					smallest = value->entityFromComponentIndices_;
				}
				});

			std::for_each(std::execution::par, smallest.begin(), smallest.end(), [&](auto entityIndex) {
				const bool allValid = ((std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex] != 0xFFFF'FFFF) && ...);
				if (!allValid) return;

				fun(entities_[sparse_[entityIndex]], std::get<SparseSet<Components>*>(pools)->dense_[std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex]]...);
				});

			// Remove tags, NOT SAFE IN PARRALEL
			std::for_each(std::execution::seq, smallest.begin(), smallest.end(), [&](auto entityIndex) {
				(RemoveTag<Components>(entities_[sparse_[entityIndex]]), ...);
				});
		}

		template<typename... Components, typename F, typename T>
		void ParForEachAddTag(F fun, T tag) {
			std::tuple pools{ GetPool<Components>()... };

			// Get the smallest pool to iterate from as a base
			std::vector<uint32_t> smallest = std::get<0>(pools)->entityFromComponentIndices_;
			ForEachTuple(pools, [&](auto&& value) {
				if (value->entityFromComponentIndices_.size() < smallest.size()) {
					smallest = value->entityFromComponentIndices_;
				}
				});

			std::for_each(std::execution::par, smallest.begin(), smallest.end(), [&](auto entityIndex) {
				const bool allValid = ((std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex] != 0xFFFF'FFFF) && ...);
				if (!allValid) return;

				fun(entities_[sparse_[entityIndex]], std::get<SparseSet<Components>*>(pools)->dense_[std::get<SparseSet<Components>*>(pools)->sparse_[entityIndex]]...);
				});

			// Remove tags, NOT SAFE IN PARRALEL
			std::for_each(std::execution::seq, smallest.begin(), smallest.end(), [&](auto entityIndex) {
				(RemoveTag<Components>(entities_[sparse_[entityIndex]]), ...);
				AddTag<T>(entities_[sparse_[entityIndex]]);
				});
		}

		uint32_t GetEntityVersion(const EntityID id) {
			return entityVersions_[id.version];
		}

	private:
		uint32_t nextEntityId_ = 0;

		std::vector<uint32_t> sparse_ = std::vector<uint32_t>(INIT_MAX_ENTITIES, 0xFFFF'FFFF);
		std::vector<uint32_t> entityVersions_ = std::vector<uint32_t>(INIT_MAX_ENTITIES, 1);
		std::vector<Entity> entities_;

		// Holds free entity indices
		std::deque<uint32_t> entityFreeList_;

		// Constructor
		EntityManager();
		// Delete copy constructor and assignment operator
		EntityManager(const EntityManager&) = delete;
		EntityManager& operator= (const EntityManager&) = delete;

		template<typename C>
		SparseSet<C>* GetPool() {
			size_t componentID = ComponentID::GetUnique<C>();
			assert(componentPools_[componentID].sparseSet != nullptr && "Failed to get pool: Component isn't registered with the entity manager\n");
			return reinterpret_cast<SparseSet<C>*>(componentPools_[componentID].sparseSet);
		}

		template<typename... Components>
		std::vector<uint32_t>& SmallestPoolEntityFromComponentIndices() {
			std::size_t sizes[]{ GetPool<Components>()->dense_.size()... };
			std::vector<uint32_t>* vectors[]{ &GetPool<Components>()->entityFromComponentIndices_... };

			auto smallest_size_ptr = std::min_element(std::begin(sizes), std::end(sizes));
			return *vectors[smallest_size_ptr - sizes];
		}

		template<typename Tuple, typename F, std::size_t... I>
		constexpr F ForEachTupleImpl(Tuple&& t, F&& f, std::index_sequence<I...>) {
			return (void)std::initializer_list<int>{(std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))), 0)...}, f;
		}

		template<typename Tuple, typename F>
		constexpr F ForEachTuple(Tuple&& t, F&& f) {
			return ForEachTupleImpl(std::forward<Tuple>(t), std::forward<F>(f),
				std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
		}

		template<typename C>
		void RemoveTag(Entity entity) {
			if constexpr (std::is_base_of_v<EventTagComponent, C>) {
				entity.RemoveComponent<C>();
			}
		}

		template<typename C>
		void AddTag(Entity entity) {
			if constexpr (std::is_base_of_v<EventTagComponent, C>) {
				entity.AddComponent<C>();
			}
		}
	};

	template<typename T>
	inline void Entity::AddComponent() {
		EntityManager::Instance().AddComponent<T>(*this);
	}

	// Aggregate initialization
	template<typename T>
	inline void Entity::AddComponent(T&& t) {
		EntityManager::Instance().AddComponent<T>(*this, std::move(t));
	}

	template<typename... T>
	inline void Entity::AddComponents() {
		(EntityManager::Instance().AddComponent<T>(*this), ...);
	}

	template<typename T>
	inline T* const Entity::GetComponentPtr() {
		return EntityManager::Instance().GetComponentPtr<T>(*this);
	}

	template<typename T>
	bool Entity::HasComponent() {
		return EntityManager::Instance().HasComponent<T>(*this);
	}

	template<typename T>
	inline void Entity::RemoveComponent() {
		EntityManager::Instance().RemoveComponent<T>(*this);
	}
}