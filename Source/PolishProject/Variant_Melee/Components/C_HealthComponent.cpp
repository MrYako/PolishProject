#include "C_HealthComponent.h"

UC_HealthComponent::UC_HealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UC_HealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
}

void UC_HealthComponent::ApplyDamage(float Amount, AActor* DamageInstigator)
{
	if (!IsAlive() || Amount <= 0.f) return;

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Amount);
	OnDamageReceived.Broadcast(Amount, DamageInstigator);

	if (!IsAlive())
	{
		OnDeath.Broadcast();
	}
}

float UC_HealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f;
}

void UC_HealthComponent::Reset()
{
	CurrentHealth = MaxHealth;
}