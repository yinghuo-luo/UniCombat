// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AtlasSaveVersionPolicy.generated.h"

/*
* @EditInlineNew 这个类的对象可以在属性面板里内联创建也就是当它作为某个 UPROPERTY 的类型时，
* 编辑器可以直接在 Details 面板里点 + / New 来创建一个实例，而不是必须先单独做成资产再引用
* 即：允许从属性窗口创建该类的新实例
* 
* @DefaultToInstanced 这个类在被当作子对象引用时，默认倾向于按“实例对象”处理
* 它常和 EditInlineNew、属性上的 Instanced 搭配，用于让每个拥有者持有自己独立的一份子对象实例，
* 而不是共享同一个模板引用
* 该类的所有实例都被视为“instanced”，并且它会继承到子类。
通俗说，DefaultToInstanced 的目标是让这个类更适合扮演“嵌在别的对象里面的小对象”

这是一个可扩展的策略对象基类，可以被做成蓝图子类，并且适合在别的对象属性里直接内联创建成独立实例
 */

/**
 * 版本策略
 */
UCLASS(Blueprintable,EditInlineNew,DefaultToInstanced)
class UNICOMBAT_API UAtlasSaveVersionPolicy : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent,Category= "Save| Version")
	int32 GetCurrentSaveVersion()const;

	UFUNCTION(BlueprintNativeEvent, Category = "Save|Version")
	bool CanLoadHeader(const FAtlasSaveHeader& Header, FString& OutFailureReason) const;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Save|Version")
	bool CanLoadProviderRecord(const FAtlasProviderRecord& Record, FString& OutFailureReason) const;
protected:
	int32 ResolveCurrentSaveVersion()const;
	int32 ResolveMinimumLoadableSaveVersion() const;
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Save|Version",meta = (ClampMin = "0",UIMin = "0"))
	int32 CurrentSaveVersionOverride = 0;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category= "Save|Version",meta = (ClampMin = "0",UIMin = "0"))
	int32 MinimumLoadableSaveVersionOverride = 0;
};
