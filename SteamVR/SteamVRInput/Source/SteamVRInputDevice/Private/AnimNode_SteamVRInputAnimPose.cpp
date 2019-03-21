#include "AnimNode_SteamVRInputAnimPose.h"
#include "ISteamVRInputDeviceModule.h"
#include "AnimationRuntime.h"
#include "AnimInstanceProxy.h"
#include "SteamVRInputDevice.h"


FAnimNode_SteamVRInputAnimPose::FAnimNode_SteamVRInputAnimPose()
{
}

void FAnimNode_SteamVRInputAnimPose::Initialize(const FAnimationInitializeContext& Context)
{
	// Setup any retargetting here
	TransformedBoneNames.Reserve( SteamVRSkeleton::GetBoneCount() );
	for ( int32 boneIndex = 0; boneIndex < SteamVRSkeleton::GetBoneCount(); ++boneIndex )
	{
		const FName& SrcBoneName = SteamVRSkeleton::GetBoneName( boneIndex );

		// Prep for retargetting to UE Hand
		FName* TargetBoneName = BoneNameMap.Find(SrcBoneName);
		if (TargetBoneName == nullptr)
		{
			FName NewName = SrcBoneName;		// TODO: Remap to UE4 Hand
			TransformedBoneNames.Add(NewName);
			BoneNameMap.Add(SrcBoneName, NewName);
		}
		else
		{
			TransformedBoneNames.Add(*TargetBoneName);
		}
	}
}

void FAnimNode_SteamVRInputAnimPose::CacheBones(const FAnimationCacheBonesContext & Context)
{
}

void FAnimNode_SteamVRInputAnimPose::Update(const FAnimationUpdateContext & Context)
{
	EvaluateGraphExposedInputs.Execute(Context);
}

void FAnimNode_SteamVRInputAnimPose::Evaluate(FPoseContext& Output)
{
	Output.ResetToRefPose();

	// Update Skeletal Animation
	FSteamVRInputDevice* SteamVRInputDevice = GetSteamVRInputDevice();
	if (SteamVRInputDevice != nullptr)
	{
		VRBoneTransform_t OutPose[STEAMVR_SKELETON_BONE_COUNT];
		VRBoneTransform_t ReferencePose[STEAMVR_SKELETON_BONE_COUNT];
		FillHandTransforms(SteamVRInputDevice, OutPose, ReferencePose);

		for (int32 i = 0; i < TransformedBoneNames.Num(); ++i)
		{
			FTransform BoneTransform = FTransform();
			FName BoneName = TransformedBoneNames[i];
			GetBoneTransform(i, BoneTransform);

			int32 MeshIndex;
			if (HandSkeleton == EHandSkeleton::VR_CustomSkeleton)
			{
				MeshIndex = Output.Pose.GetBoneContainer().GetPoseBoneIndexForBoneName(BoneName);
			}
			else if (HandSkeleton == EHandSkeleton::VR_UE4HandSkeleton)
			{
				MeshIndex = i; // TODO: Implement
			}
			else
			{
				MeshIndex = i;
			}
			
			if (MeshIndex != INDEX_NONE)
			{
				FCompactPoseBoneIndex BoneIndex = Output.Pose.GetBoneContainer().MakeCompactPoseIndex(FMeshPoseBoneIndex(MeshIndex));
				if (BoneIndex != INDEX_NONE)
				{
					FQuat NewRotation;
					if (BoneTransform.GetRotation().Equals(FQuat()) ||
						BoneTransform.GetRotation().ContainsNaN())
					{
						NewRotation = Output.Pose[BoneIndex].GetRotation();
					}
					else
					{
						NewRotation = BoneTransform.GetRotation();
					}

					FVector NewTranslation;
					if (BoneTransform.GetLocation() == FVector::ZeroVector ||
						BoneTransform.ContainsNaN())
					{
						NewTranslation = Output.Pose[BoneIndex].GetTranslation();
					}
					else
					{
						NewTranslation = BoneTransform.GetLocation();
					}
					//UE_LOG(LogTemp, Warning, TEXT("[Current Translate %s] [Bone Translate %s]"), *Output.Pose[BoneIndex].GetTranslation().ToString(), *(BoneTransform.GetLocation()).ToString());

					FTransform OutTransform = FTransform(Output.Pose[BoneIndex].GetRotation(), Output.Pose[BoneIndex].GetTranslation(), Output.Pose[BoneIndex].GetScale3D());
					OutTransform.SetLocation(NewTranslation);
					OutTransform.SetRotation(NewRotation);

					// Set new bone transform
					Output.Pose[BoneIndex] = OutTransform;
				}
			}
		}

	}
}

void FAnimNode_SteamVRInputAnimPose::GetBoneTransform(int32 SteamVRBoneIndex, FTransform& OutTransform)
{
	if (Hand == EHand::VR_RightHand)
	{
		switch ( (ESteamVRBone)SteamVRBoneIndex )
		{
		case ESteamVRBone::EBone_Root:
			//OutTransform = RightHand.Root;
			break;

		case ESteamVRBone::EBone_Wrist:
			OutTransform = RightHand.Wrist;
			OutTransform.SetLocation(FVector::ZeroVector);
			OutTransform.SetRotation(FQuat((FRotator(OutTransform.GetRotation()).Add(180.f, 0.f, 45.f))));
			break;

		case ESteamVRBone::EBone_Thumb1:
			OutTransform = RightHand.Thumb_0;
			break;

		case ESteamVRBone::EBone_Thumb2:
			OutTransform = RightHand.Thumb_1;
			break;

		case ESteamVRBone::EBone_Thumb3:
			OutTransform = RightHand.Thumb_2;
			break;

		case ESteamVRBone::EBone_Thumb4:
			OutTransform = RightHand.Thumb_3;
			break;

		case ESteamVRBone::EBone_IndexFinger0:
			OutTransform = RightHand.Index_0;
			break;

		case ESteamVRBone::EBone_IndexFinger1:
			OutTransform = RightHand.Index_1;
			break;

		case ESteamVRBone::EBone_IndexFinger2:
			OutTransform = RightHand.Index_2;
			break;

		case ESteamVRBone::EBone_IndexFinger3:
			OutTransform = RightHand.Index_3;
			break;

		case ESteamVRBone::EBone_IndexFinger4:
			OutTransform = RightHand.Index_4;
			break;

		case ESteamVRBone::EBone_MiddleFinger0:
			OutTransform = RightHand.Middle_0;
			break;

		case ESteamVRBone::EBone_MiddleFinger1:
			OutTransform = RightHand.Middle_1;
			break;

		case ESteamVRBone::EBone_MiddleFinger2:
			OutTransform = RightHand.Middle_2;
			break;

		case ESteamVRBone::EBone_MiddleFinger3:
			OutTransform = RightHand.Middle_3;
			break;

		case ESteamVRBone::EBone_MiddleFinger4:
			OutTransform = RightHand.Middle_4;
			break;

		case ESteamVRBone::EBone_RingFinger0:
			OutTransform = RightHand.Ring_0;
			break;

		case ESteamVRBone::EBone_RingFinger1:
			OutTransform = RightHand.Ring_1;
			break;

		case ESteamVRBone::EBone_RingFinger2:
			OutTransform = RightHand.Ring_2;
			break;

		case ESteamVRBone::EBone_RingFinger3:
			OutTransform = RightHand.Ring_3;
			break;

		case ESteamVRBone::EBone_RingFinger4:
			OutTransform = RightHand.Ring_4;
			break;

		case ESteamVRBone::EBone_PinkyFinger0:
			OutTransform = RightHand.Pinky_0;
			break;

		case ESteamVRBone::EBone_PinkyFinger1:
			OutTransform = RightHand.Pinky_1;
			break;

		case ESteamVRBone::EBone_PinkyFinger2:
			OutTransform = RightHand.Pinky_2;
			break;

		case ESteamVRBone::EBone_PinkyFinger3:
			OutTransform = RightHand.Pinky_3;
			break;

		case ESteamVRBone::EBone_PinkyFinger4:
			OutTransform = RightHand.Pinky_4;
			break;

		default:
			break;
		}
	}
	else
	{
		switch ( (ESteamVRBone)SteamVRBoneIndex )
		{
		case ESteamVRBone::EBone_Root:
			//OutTransform = LeftHand.Root;
			break;

		case ESteamVRBone::EBone_Wrist:
			OutTransform = LeftHand.Wrist;
			OutTransform.SetLocation(FVector::ZeroVector);
			OutTransform.SetRotation(FQuat((FRotator(OutTransform.GetRotation()).Add(180.f, 0.f, 45.f))));
			break;

		case ESteamVRBone::EBone_Thumb1:
			OutTransform = LeftHand.Thumb_0;
			break;

		case ESteamVRBone::EBone_Thumb2:
			OutTransform = LeftHand.Thumb_1;
			break;

		case ESteamVRBone::EBone_Thumb3:
			OutTransform = LeftHand.Thumb_2;
			break;

		case ESteamVRBone::EBone_Thumb4:
			OutTransform = LeftHand.Thumb_3;
			break;

		case ESteamVRBone::EBone_IndexFinger0:
			OutTransform = LeftHand.Index_0;
			break;

		case ESteamVRBone::EBone_IndexFinger1:
			OutTransform = LeftHand.Index_1;
			break;

		case ESteamVRBone::EBone_IndexFinger2:
			OutTransform = LeftHand.Index_2;
			break;

		case ESteamVRBone::EBone_IndexFinger3:
			OutTransform = LeftHand.Index_3;
			break;

		case ESteamVRBone::EBone_IndexFinger4:
			OutTransform = LeftHand.Index_4;
			break;

		case ESteamVRBone::EBone_MiddleFinger0:
			OutTransform = LeftHand.Middle_0;
			break;

		case ESteamVRBone::EBone_MiddleFinger1:
			OutTransform = LeftHand.Middle_1;
			break;

		case ESteamVRBone::EBone_MiddleFinger2:
			OutTransform = LeftHand.Middle_2;
			break;

		case ESteamVRBone::EBone_MiddleFinger3:
			OutTransform = LeftHand.Middle_3;
			break;

		case ESteamVRBone::EBone_MiddleFinger4:
			OutTransform = LeftHand.Middle_4;
			break;

		case ESteamVRBone::EBone_RingFinger0:
			OutTransform = LeftHand.Ring_0;
			break;

		case ESteamVRBone::EBone_RingFinger1:
			OutTransform = LeftHand.Ring_1;
			break;

		case ESteamVRBone::EBone_RingFinger2:
			OutTransform = LeftHand.Ring_2;
			break;

		case ESteamVRBone::EBone_RingFinger3:
			OutTransform = LeftHand.Ring_3;
			break;

		case ESteamVRBone::EBone_RingFinger4:
			OutTransform = LeftHand.Ring_4;
			break;

		case ESteamVRBone::EBone_PinkyFinger0:
			OutTransform = LeftHand.Pinky_0;
			break;

		case ESteamVRBone::EBone_PinkyFinger1:
			OutTransform = LeftHand.Pinky_1;
			break;

		case ESteamVRBone::EBone_PinkyFinger2:
			OutTransform = LeftHand.Pinky_2;
			break;

		case ESteamVRBone::EBone_PinkyFinger3:
			OutTransform = LeftHand.Pinky_3;
			break;

		case ESteamVRBone::EBone_PinkyFinger4:
			OutTransform = LeftHand.Pinky_4;
			break;

		default:
			break;
		}
	}

	//
}

void FAnimNode_SteamVRInputAnimPose::FillHandTransforms(FSteamVRInputDevice* SteamVRInputDevice, VRBoneTransform_t* OutPose, VRBoneTransform_t* ReferencePose)
{
	if (SteamVRInputDevice != nullptr)
	{
		// Setup Motion Range
		EVRSkeletalMotionRange SteamVRMotionRange = (MotionRange == EMotionRange::VR_WithController) ? VRSkeletalMotionRange_WithController : VRSkeletalMotionRange_WithoutController;

		if (Hand == EHand::VR_LeftHand)
		{
			// Left Hand - Grab Skeletal Data
			if (SteamVRInputDevice->GetSkeletalData(true, SteamVRMotionRange, OutPose, ReferencePose))
			{
				LeftHand.Root = GetUETransform(OutPose[0], ReferencePose[0]);
				LeftHand.Wrist = GetUETransform(OutPose[1], ReferencePose[1]);

				LeftHand.Thumb_0 = GetUETransform(OutPose[2], ReferencePose[2]);
				LeftHand.Thumb_1 = GetUETransform(OutPose[3], ReferencePose[3]);
				LeftHand.Thumb_2 = GetUETransform(OutPose[4], ReferencePose[4]);
				LeftHand.Thumb_3 = GetUETransform(OutPose[5], ReferencePose[5]);

				LeftHand.Index_0 = GetUETransform(OutPose[6], ReferencePose[6]);
				LeftHand.Index_1 = GetUETransform(OutPose[7], ReferencePose[7]);
				LeftHand.Index_2 = GetUETransform(OutPose[8], ReferencePose[8]);
				LeftHand.Index_3 = GetUETransform(OutPose[9], ReferencePose[9]);
				LeftHand.Index_4 = GetUETransform(OutPose[10], ReferencePose[10]);

				LeftHand.Middle_0 = GetUETransform(OutPose[11], ReferencePose[11]);
				LeftHand.Middle_1 = GetUETransform(OutPose[12], ReferencePose[12]);
				LeftHand.Middle_2 = GetUETransform(OutPose[13], ReferencePose[13]);
				LeftHand.Middle_3 = GetUETransform(OutPose[14], ReferencePose[14]);
				LeftHand.Middle_4 = GetUETransform(OutPose[15], ReferencePose[15]);

				LeftHand.Ring_0 = GetUETransform(OutPose[16], ReferencePose[16]);
				LeftHand.Ring_1 = GetUETransform(OutPose[17], ReferencePose[17]);
				LeftHand.Ring_2 = GetUETransform(OutPose[18], ReferencePose[18]);
				LeftHand.Ring_3 = GetUETransform(OutPose[19], ReferencePose[19]);
				LeftHand.Ring_4 = GetUETransform(OutPose[20], ReferencePose[20]);

				LeftHand.Pinky_0 = GetUETransform(OutPose[21], ReferencePose[21]);
				LeftHand.Pinky_1 = GetUETransform(OutPose[22], ReferencePose[22]);
				LeftHand.Pinky_2 = GetUETransform(OutPose[23], ReferencePose[23]);
				LeftHand.Pinky_3 = GetUETransform(OutPose[24], ReferencePose[24]);
				LeftHand.Pinky_4 = GetUETransform(OutPose[25], ReferencePose[25]);

				LeftHand.Aux_Thumb = GetUETransform(OutPose[26], ReferencePose[26]);
				LeftHand.Aux_Index = GetUETransform(OutPose[27], ReferencePose[27]);
				LeftHand.Aux_Middle = GetUETransform(OutPose[28], ReferencePose[28]);
				LeftHand.Aux_Ring = GetUETransform(OutPose[29], ReferencePose[29]);
				LeftHand.Aux_Pinky = GetUETransform(OutPose[30], ReferencePose[30]);

				LeftHand.Bone_Count = GetUETransform(OutPose[31], ReferencePose[31]);
			}
		}
		else
		{
			// Right Hand - Grab Skeletal Data
			SteamVRInputDevice->GetSkeletalData(false, SteamVRMotionRange, OutPose, ReferencePose);

			RightHand.Root = GetUETransform(OutPose[0], ReferencePose[0]);
			RightHand.Wrist = GetUETransform(OutPose[1], ReferencePose[1]);

			RightHand.Thumb_0 = GetUETransform(OutPose[2], ReferencePose[2]);
			RightHand.Thumb_1 = GetUETransform(OutPose[3], ReferencePose[3]);
			RightHand.Thumb_2 = GetUETransform(OutPose[4], ReferencePose[4]);
			RightHand.Thumb_3 = GetUETransform(OutPose[5], ReferencePose[5]);

			RightHand.Index_0 = GetUETransform(OutPose[6], ReferencePose[6]);
			RightHand.Index_1 = GetUETransform(OutPose[7], ReferencePose[7]);
			RightHand.Index_2 = GetUETransform(OutPose[8], ReferencePose[8]);
			RightHand.Index_3 = GetUETransform(OutPose[9], ReferencePose[9]);
			RightHand.Index_4 = GetUETransform(OutPose[10], ReferencePose[10]);

			RightHand.Middle_0 = GetUETransform(OutPose[11], ReferencePose[11]);
			RightHand.Middle_1 = GetUETransform(OutPose[12], ReferencePose[12]);
			RightHand.Middle_2 = GetUETransform(OutPose[13], ReferencePose[13]);
			RightHand.Middle_3 = GetUETransform(OutPose[14], ReferencePose[14]);
			RightHand.Middle_4 = GetUETransform(OutPose[15], ReferencePose[15]);

			RightHand.Ring_0 = GetUETransform(OutPose[16], ReferencePose[16]);
			RightHand.Ring_1 = GetUETransform(OutPose[17], ReferencePose[17]);
			RightHand.Ring_2 = GetUETransform(OutPose[18], ReferencePose[18]);
			RightHand.Ring_3 = GetUETransform(OutPose[19], ReferencePose[19]);
			RightHand.Ring_4 = GetUETransform(OutPose[20], ReferencePose[20]);

			RightHand.Pinky_0 = GetUETransform(OutPose[21], ReferencePose[21]);
			RightHand.Pinky_1 = GetUETransform(OutPose[22], ReferencePose[22]);
			RightHand.Pinky_2 = GetUETransform(OutPose[23], ReferencePose[23]);
			RightHand.Pinky_3 = GetUETransform(OutPose[24], ReferencePose[24]);
			RightHand.Pinky_4 = GetUETransform(OutPose[25], ReferencePose[25]);

			RightHand.Aux_Thumb = GetUETransform(OutPose[26], ReferencePose[26]);
			RightHand.Aux_Index = GetUETransform(OutPose[27], ReferencePose[27]);
			RightHand.Aux_Middle = GetUETransform(OutPose[28], ReferencePose[28]);
			RightHand.Aux_Ring = GetUETransform(OutPose[29], ReferencePose[29]);
			RightHand.Aux_Pinky = GetUETransform(OutPose[30], ReferencePose[30]);

			RightHand.Bone_Count = GetUETransform(OutPose[31], ReferencePose[31]);
		}
	}
}

FTransform FAnimNode_SteamVRInputAnimPose::GetUETransform(VRBoneTransform_t SteamBoneTransform, VRBoneTransform_t SteamBoneReference)
{
	FTransform RetTransform;

	FQuat OrientationQuat(SteamBoneTransform.orientation.x,
		-SteamBoneTransform.orientation.y,
		 SteamBoneTransform.orientation.z,
		-SteamBoneTransform.orientation.w);
	OrientationQuat.Normalize();
	RetTransform = FTransform(OrientationQuat,
		//FVector());
		FVector(SteamBoneReference.position.v[0],
				-SteamBoneReference.position.v[1],
				SteamBoneReference.position.v[2]));

	return RetTransform;
}

FSteamVRInputDevice* FAnimNode_SteamVRInputAnimPose::GetSteamVRInputDevice()
{
	TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
	for (auto MotionController : MotionControllers)
	{
		FName DeviceName = MotionController->GetModularFeatureName();
		return static_cast<FSteamVRInputDevice*>(MotionController);
	}

	return nullptr;
}
